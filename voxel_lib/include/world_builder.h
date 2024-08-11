#pragma once

#include "voxel_lib.h"

#include <deque>
#include <mutex>
#include <functional>
#include <thread>

namespace Voxels
{
    class WorldBuilder
    {
    public:
        WorldBuilder(World& world, std::function<void(Chunk&)> func);

        ~WorldBuilder();

        void PushChunk(ChunkId chunk);

        bool PopChunk(ChunkId* chunk);

    private:
        void StartQueue();
        void ProcessQueue();

        bool PopPendingChunk(ChunkId* chunk);

        void StopQueue();

        Voxels::World& WorldMap;

        std::deque<ChunkId> PendingChunks;
        std::deque<ChunkId> CompletedChunks;

        std::function<void(Chunk&)> GenerationFunction;

        std::thread WorkerThread; // pool this?

        std::mutex RunMutex;
        std::mutex QueueMutex;

        bool RunQueue = false;
    };
}