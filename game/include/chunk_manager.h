#pragma once

#include "voxel_lib.h"
#include "world_builder.h"
#include "chunk_mesher.h"

#include <vector>
#include <functional>


class ChunkLoop
{
public:
    ChunkLoop(int size);

    void Fill(Voxels::ChunkId center);

    void DoForEach(std::function<void(Voxels::ChunkId)> func);
private:
    int Size = 1;

    std::vector<Voxels::ChunkId> TopRow;
    std::vector<Voxels::ChunkId> BottomRow;
    std::vector<Voxels::ChunkId> LeftColumn;
    std::vector<Voxels::ChunkId> RightColumn;
};

class ChunkManager
{
public:
    ChunkManager(Voxels::World& map);

    void Update(const Vector3& position);

    void Abort();

    void DrawDebug3D();
    void DrawDebug2D();

    void DoForEachRenderChunk(std::function<void(Voxels::Chunk*)> func);

    Voxels::WorldBuilder Builder;
    Voxels::ChunkMeshTaskPool Mesher;

private:
    Voxels::World& Map;

    Voxels::ChunkId CurrentChunk;

    std::vector<ChunkLoop> RenderArea;
    std::vector<ChunkLoop> LoadedArea;

    std::set<uint64_t> ChunksWithMeshes;
    std::set<uint64_t> PendingMeshUnloads;

    static constexpr int RenderDistance = 5;
    static constexpr int LoadDistance = 4;

    Vector3                     WorldSpacePosition = { 0 };

    void ValidateChunkGeneration(Voxels::ChunkId id);
    void ValidateChunkMesh(Voxels::ChunkId id);

    void DrawDebugChunk(Voxels::ChunkId id, Color tint);
};