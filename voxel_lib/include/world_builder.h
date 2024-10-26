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

        void SetTerrainGenerationFunction(std::function<void(Chunk&)> func);
        void SetPopulateFunction(std::function<void(Chunk&)> func);

        void Abort();

        void PushChunk(ChunkId chunk);

        bool PopChunk(ChunkId* chunk);

    private:
        void StartQueue();
        void ProcessQueue();

        bool RunOneTask();

        bool PopPendingChunk(ChunkId* chunk);
        bool PopPendingPopulationChunk(ChunkId* chunk);

        void StopQueue();

        Voxels::World& WorldMap;

        std::deque<ChunkId> PendingChunks;
        std::set<uint64_t> ProcessingChunks;

        std::list<ChunkId> PendingPopulationChunks;
        std::deque<ChunkId> CompletedChunks;

        std::function<void(Chunk&)> TerrainGenerationFunction;
        std::function<void(Chunk&)> PopulationGenerationFunction;

        std::thread WorkerThread; // pool this?

        std::mutex RunMutex;
        std::mutex QueueMutex;

        bool RunQueue = false;
    };
}