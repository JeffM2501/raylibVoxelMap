#include "world_builder.h"

namespace Voxels
{
    WorldBuilder::WorldBuilder(World& world, std::function<void(Chunk&)> func)
        : WorldMap(world)
        , GenerationFunction(func)
    {
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
        return true;
    }

    void WorldBuilder::StopQueue()
    {
        std::lock_guard guard(RunMutex);
        RunQueue = false;
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

            ChunkId processChunk;

            if (!PopPendingChunk(&processChunk))
            {
                StopQueue();
                return;
            }
   
            auto &chunk = WorldMap.AddChunk(processChunk.Coordinate.h, processChunk.Coordinate.v);
            if (chunk.GetStatus() != ChunkStatus::Empty)
                continue;

            GenerationFunction(chunk);

            std::lock_guard outBoundGuard(QueueMutex);
            CompletedChunks.push_back(processChunk);
        }
    }
}
