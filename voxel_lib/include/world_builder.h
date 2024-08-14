#pragma once

#include "voxel_lib.h"

#include <deque>
#include <mutex>
#include <functional>
#include <set>
#include <thread>

namespace Voxels
{
    class WorldBuilder
    {
    public:
        WorldBuilder(World& world);

        ~WorldBuilder();

        void SetGenerationFunction(std::function<void(Chunk&)> func);

        void Abort();

        void PushChunk(ChunkId chunk);

        bool PopChunk(ChunkId* chunk);

    private:
        void StartQueue();
        void ProcessQueue();

        bool PopPendingChunk(ChunkId* chunk);

        void StopQueue();

        Voxels::World& WorldMap;

        std::deque<ChunkId> PendingChunks;
        std::set<uint64_t> ProcessingChunks;

        std::deque<ChunkId> CompletedChunks;

        std::function<void(Chunk&)> GenerationFunction;

        std::thread WorkerThread; // pool this?

        std::mutex RunMutex;
        std::mutex QueueMutex;

        bool RunQueue = false;
    };
}