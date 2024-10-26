#include "world_builder.h"

#include "tasks.h"

namespace Voxels
{
    WorldBuilder::WorldBuilder(World& world)
        : WorldMap(world)
    {
    }

    void WorldBuilder::SetTerrainGenerationFunction(std::function<void(Chunk&)> func)
    {
        TerrainGenerationFunction = func;
    }

    void WorldBuilder::SetPopulateFunction(std::function<void(Chunk&)> func)
    {
        PopulationGenerationFunction = func;
    }

    WorldBuilder::~WorldBuilder()
    {
        Abort();
    }

    void WorldBuilder::Abort()
    {
        {
            std::lock_guard guard(RunMutex);
            RunQueue = false;
        }
        if (WorkerThread.joinable())
            WorkerThread.join();
    }

    void WorldBuilder::PushChunk(ChunkId chunk)
    {
        {
            std::lock_guard guard(QueueMutex);
            
            auto itr = ProcessingChunks.find(chunk.Id);
            if (itr != ProcessingChunks.end())
                return;

            ProcessingChunks.insert(chunk.Id);
            PendingChunks.push_back(chunk);

        }
        StartQueue();
    }

    bool WorldBuilder::PopChunk(ChunkId* chunk)
    {
        std::lock_guard guard(QueueMutex);
        if (CompletedChunks.empty() || !chunk)
            return false;

        *chunk = CompletedChunks.front();
        CompletedChunks.pop_front();
        return true;
    }

    void WorldBuilder::StartQueue()
    {
        Tasks::AddTask([this]() { RunOneTask(); });
        return;

        std::lock_guard guard(RunMutex);

        if (RunQueue)
            return;

        if (WorkerThread.joinable())
            WorkerThread.join();
       
        RunQueue = true;
        WorkerThread = std::thread([this]() {ProcessQueue(); });
    }

    bool WorldBuilder::PopPendingChunk(ChunkId* chunk)
    {
        std::lock_guard guard(QueueMutex);
        if (PendingChunks.empty() || !chunk)
            return false;

        *chunk = PendingChunks.front();
        PendingChunks.pop_front();
        auto pending = ProcessingChunks.find(chunk->Id);
        if (pending != ProcessingChunks.end())
            ProcessingChunks.erase(pending);

        return true;
    }

    bool WorldBuilder::PopPendingPopulationChunk(ChunkId* chunk)
    {
        std::lock_guard guard(QueueMutex);
        if (PendingPopulationChunks.empty() || !chunk)
            return false;

        for (auto itr = PendingPopulationChunks.begin(); itr != PendingPopulationChunks.end(); itr++)
        {
            if (WorldMap.SurroundingChunksGenerated(*itr))
            {
                *chunk = *itr;
                PendingPopulationChunks.erase(itr);
                return true;
            }
        }
        return false;
    }

    void WorldBuilder::StopQueue()
    {
        std::lock_guard guard(RunMutex);
        RunQueue = false;
    }

    bool WorldBuilder::RunOneTask()
    {
        ChunkId processChunk;

        bool didSomething = false;

        if (PopPendingPopulationChunk(&processChunk))
        {
            auto* chunk = WorldMap.GetChunk(processChunk.Coordinate.h, processChunk.Coordinate.v);

            if (PopulationGenerationFunction)
                PopulationGenerationFunction(*chunk);

            chunk->SetStatus(ChunkStatus::Populated);

            std::lock_guard outBoundGuard(QueueMutex);
            CompletedChunks.push_back(processChunk);

            didSomething = true;
        }

        if (PopPendingChunk(&processChunk))
        {
            auto& chunk = WorldMap.AddChunk(processChunk.Coordinate.h, processChunk.Coordinate.v);
            if (chunk.GetStatus() != ChunkStatus::Empty)
                return true;

            chunk.SetStatus(ChunkStatus::Generating);
            TerrainGenerationFunction(chunk);
            chunk.SetStatus(ChunkStatus::Generated);

            if (WorldMap.SurroundingChunksGenerated(processChunk))
            {
                if (PopulationGenerationFunction)
                    PopulationGenerationFunction(chunk);

                chunk.SetStatus(ChunkStatus::Populated);

                std::lock_guard outBoundGuard(QueueMutex);
                CompletedChunks.push_back(processChunk);
            }
            else
            {
                PendingPopulationChunks.push_back(processChunk);
            }
            didSomething = true;
        }

        return didSomething;
    }

    void WorldBuilder::ProcessQueue()
    {
        bool run = true;

        while (run)
        {
            {
                std::lock_guard guard(RunMutex);
                if (!RunQueue)
                {
                    RunQueue = false;
                    return;
                }
            }

            if (!RunOneTask())
            {
                StopQueue();
                return;
            }
        }
    }
}
