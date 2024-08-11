#pragma once

#include "geometry_builder.h"
#include "voxel_lib.h"

#include <mutex>
#include <thread>
#include <deque>
#include <list>

namespace Voxels
{
    class ChunkMesher
    {
    public:
        enum class Status
        {
            Unbuilt,
            Building,
            Built,
        };

        ChunkMesher(World& world, ChunkId chunk);

        void BuildMesh();

        Mesh GetMesh();

        Status GetStatus() const;

    private:
        World&              Map;
        ChunkId             MapChunk;
        CubeGeometryBuilder Builder;
        Mesh                ChunkMesh = { 0 };
        Status              BuildStatus = Status::Unbuilt;

        mutable std::mutex  StatusLock;

        void SetStatus(Status status);

        int GetChunkFaceCount();
    };

    class ChunkMeshTaskPool
    {
    public:
        ChunkMeshTaskPool(World& world);
        ~ChunkMeshTaskPool();

        void PushChunk(ChunkId chunk);
        bool PopChunk(ChunkId* chunk);
    
    private:
        void StartQueue();
        void ProcessQueue();

        bool PopPendingChunk(ChunkId* chunk);

        void StopQueue();

        World& Map;

        std::thread WorkerThread; // pool this?

        std::mutex RunMutex;
        std::mutex QueueMutex;

        bool RunQueue = false;

        std::list<ChunkId> PendingChunks;
        std::deque<ChunkId> CompletedChunks;
    };
}