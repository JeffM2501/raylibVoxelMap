#include "chunk_mesher.h"

#include "tasks.h"

namespace Voxels
{
    ChunkMesher::ChunkMesher(World& world, ChunkId chunk)
        : Map(world)
        , MapChunk(chunk)
        , Builder(ChunkMesh)
    {
        SetStatus(Status::Unbuilt);
    }

    void ChunkMesher::BuildMesh()
    {
        SetStatus(Status::Building);

        Builder.Allocate(GetChunkFaceCount());

        size_t count = 0;
        for (int d = 0; d < Chunk::ChunkHeight; d++)
        {
            for (int v = 0; v < Chunk::ChunkSize; v++)
            {
                for (int h = 0; h < Chunk::ChunkSize; h++)
                {
                    if (!Map.BlockIsSolid(MapChunk, h, v, d))
                        continue;

                    // build up the list of faces that this block needs
                    bool faces[6] = { false, false, false, false, false, false };

                    if (!Map.BlockIsSolid(MapChunk, h - 1, v, d))
                        faces[EastFace] = true;

                    if (!Map.BlockIsSolid(MapChunk, h + 1, v, d))
                        faces[WestFace] = true;

                    if (!Map.BlockIsSolid(MapChunk, h, v - 1, d))
                        faces[NorthFace] = true;

                    if (!Map.BlockIsSolid(MapChunk, h, v + 1, d))
                        faces[SouthFace] = true;

                    if (!Map.BlockIsSolid(MapChunk, h, v, d + 1))
                        faces[UpFace] = true;

                    if (!Map.BlockIsSolid(MapChunk, h, v, d - 1))
                        faces[DownFace] = true;

                    // build the faces that hit open air for this voxel block
                    Builder.AddCube(Vector3{ (float)h, (float)d, (float)v }, faces, Map.GetVoxel(MapChunk,h, v, d));
                }
            }
        }

        SetStatus(Status::Built);
    }

    int ChunkMesher::GetChunkFaceCount()
    {
        int count = 0;
        for (int d = 0; d < Chunk::ChunkHeight; d++)
        {
            for (int v = 0; v < Chunk::ChunkSize; v++)
            {
                for (int h = 0; h < Chunk::ChunkSize; h++)
                {
                    if (!Map.BlockIsSolid(MapChunk, h, v, d))
                        continue;

                    if (!Map.BlockIsSolid(MapChunk, h + 1, v, d))
                        count++;

                    if (!Map.BlockIsSolid(MapChunk, h - 1, v, d))
                        count++;

                    if (!Map.BlockIsSolid(MapChunk, h, v + 1, d))
                        count++;

                    if (!Map.BlockIsSolid(MapChunk, h, v - 1, d))
                        count++;

                    if (!Map.BlockIsSolid(MapChunk, h, v, d + 1))
                        count++;

                    if (!Map.BlockIsSolid(MapChunk, h, v, d - 1))
                        count++;
                }
            }
        }

        return count;
    }

    Mesh ChunkMesher::GetMesh()
    {
        return ChunkMesh;
    }

    ChunkMesher::Status ChunkMesher::GetStatus() const
    {
        std::lock_guard guard(StatusLock);
        return BuildStatus;
    }

    void ChunkMesher::SetStatus(ChunkMesher::Status status)
    {
        std::lock_guard guard(StatusLock);
        BuildStatus = status;
    }

    ChunkMeshTaskPool::ChunkMeshTaskPool(World& world)
        : Map(world)
    {

    }

    ChunkMeshTaskPool::~ChunkMeshTaskPool()
    {
        Abort();
    }

    void ChunkMeshTaskPool::Abort()
    {
        {
            std::lock_guard guard(RunMutex);
            RunQueue = false;
        }
        if (WorkerThread.joinable())
            WorkerThread.join();
    }

    void ChunkMeshTaskPool::PushChunk(ChunkId chunk)
    {
        {
            std::lock_guard guard(QueueMutex);
            PendingChunks.push_back(chunk);
        }
        StartQueue();
    }

    bool ChunkMeshTaskPool::PopChunk(ChunkId* chunk)
    {
        std::lock_guard guard(QueueMutex);
        if (CompletedChunks.empty() || !chunk)
            return false;

        *chunk = CompletedChunks.front();
        CompletedChunks.pop_front();
        return true;
    }

    void ChunkMeshTaskPool::StartQueue()
    {
//         Tasks::AddTask([this]()
//             {
//                 RunOneTask();
//             });
// 
//         return;
        std::lock_guard guard(RunMutex);

        if (RunQueue)
            return;

        if (WorkerThread.joinable())
            WorkerThread.join();

        RunQueue = true;
        WorkerThread = std::thread([this]() { ProcessQueue(); });
    }

    bool ChunkMeshTaskPool::RunOneTask()
    {
        ChunkId processChunk;

        if (!PopPendingChunk(&processChunk))
        {
            return false;
        }

        ChunkMesher mesher(Map, processChunk);
        mesher.BuildMesh();

        Chunk* chunk = Map.GetChunk(processChunk);

        chunk->ChunkMesh = mesher.GetMesh();
        chunk->SetStatus(ChunkStatus::Meshed);

        std::lock_guard outBoundGuard(QueueMutex);
        CompletedChunks.push_back(processChunk);

        return true;
    }

    void ChunkMeshTaskPool::ProcessQueue()
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
            }
        }
    }

    bool ChunkMeshTaskPool::PopPendingChunk(ChunkId* chunk)
    {
        std::lock_guard guard(QueueMutex);
        if (PendingChunks.empty() || !chunk)
            return false;

        for (auto itr = PendingChunks.begin(); itr != PendingChunks.end(); itr++)
        {
            if (Map.SurroundingChunksGenerated(*itr))
            {
                *chunk = *itr;
                PendingChunks.erase(itr);
                return true;
            }
        }
        return false;
    }

    bool ChunkMeshTaskPool::PendingChunksEmpty()
    {
        std::lock_guard guard(QueueMutex);
        return PendingChunks.empty();
    }

    void ChunkMeshTaskPool::StopQueue()
    {
        std::lock_guard guard(RunMutex);
        RunQueue = false;
    }

}