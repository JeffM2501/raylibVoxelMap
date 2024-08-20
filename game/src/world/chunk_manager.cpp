#include "chunk_manager.h"

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

using namespace Voxels;

ChunkLoop::ChunkLoop(int size)
{
    Size = size;
    int distance = (size * 2) + 1;

    TopRow.resize(distance);
    BottomRow.resize(distance);
    LeftColumn.resize(distance - 2);
    RightColumn.resize(distance - 2);
}

void ChunkLoop::Fill(ChunkId center)
{
    int distance = (Size * 2) + 1;

    // top and bottom rows

    int leftH = center.Coordinate.h - Size;
    int rightH = center.Coordinate.h + Size;

    int topV = center.Coordinate.v - Size;
    int bottomV = center.Coordinate.v + Size;

    for (int i = 0; i < distance; i++)
    {
        TopRow[i] = ChunkId(leftH + i, topV);
        BottomRow[i] = ChunkId(leftH + i, bottomV);
    }
    for (int i = 0; i < distance-2; i++)
    {
        LeftColumn[i] = ChunkId(leftH, topV + 1 + i);
        RightColumn[i] = ChunkId(rightH, topV + 1 + i);
    }
}

void ChunkLoop::DoForEach(std::function<void(Voxels::ChunkId)> func)
{
    for (auto& id : TopRow)
        func(id);

    for (auto& id : RightColumn)
        func(id);

    for (auto& id : BottomRow)
        func(id);

    for (auto& id : LeftColumn)
        func(id);
}

ChunkManager::ChunkManager(World& map)
    : Builder(map)
    , Mesher(map)
    , Map(map)
{
    for (int i = 0; i < RenderDistance; i++)
    {
        RenderArea.emplace_back(i + 1);
    }

    for (int i = 0; i < LoadDistance; i++)
    {
        LoadedArea.emplace_back(i + 1 + RenderDistance);
    }
}

void ChunkManager::DrawDebugChunk(Voxels::ChunkId id, Color tint)
{
    float h = float(id.Coordinate.h) * Chunk::ChunkSize;
    float v = float(id.Coordinate.v) * Chunk::ChunkSize;

    DrawCubeWires(Vector3{ h + Chunk::ChunkSize * 0.5f, Chunk::ChunkHeight * 0.5f, v + Chunk::ChunkSize * 0.5f },
        Chunk::ChunkSize - 1, Chunk::ChunkSize, Chunk::ChunkSize - 1, tint);
}

void ChunkManager::DrawDebug3D()
{
    for (auto& area : LoadedArea)
    {
        area.DoForEach([this](ChunkId id) 
            {
                DrawDebugChunk(id, ColorAlpha(MAROON, 0.25f));
            });
    }
}

void ChunkManager::DrawDebug2D()
{
    DrawText(TextFormat("Current Chunk h%d v%d", CurrentChunk.Coordinate.h, CurrentChunk.Coordinate.v), 10, GetScreenHeight()-40, 20, BLACK);
    DrawText(TextFormat("Meshes %d", int(ChunksWithMeshes.size())), 10, GetScreenHeight() - 20, 20, BLACK);
}

void ChunkManager::Update(const Vector3& position)
{
    WorldSpacePosition = position;

    ChunkId thisChunk(int(position.x / Chunk::ChunkSize), int(position.z / Chunk::ChunkSize));

    if (!CurrentChunk.IsValid() || thisChunk.Id != CurrentChunk.Id)
    {
        std::set<uint64_t> meshedChunks = ChunksWithMeshes;

        CurrentChunk = thisChunk;
        for (auto& area : RenderArea)
            area.Fill(CurrentChunk);

        for (auto& area : LoadedArea)
            area.Fill(CurrentChunk);

        // validate each ring from center out to see if it's loaded
        meshedChunks.erase(CurrentChunk.Id);
        PendingMeshUnloads.erase(CurrentChunk.Id);
        ValidateChunkGeneration(CurrentChunk);

        for (auto& area : RenderArea)
        {
            area.DoForEach([&](ChunkId id) 
                {
                    meshedChunks.erase(id.Id);
                    PendingMeshUnloads.erase(id.Id);
                    ValidateChunkMesh(id); 
                });
        }

        for (auto& area : LoadedArea)
        {
            area.DoForEach([&](ChunkId id) 
                {
                    meshedChunks.erase(id.Id);
                    PendingMeshUnloads.erase(id.Id);
                    ValidateChunkGeneration(id);
                });
        }

        // see what chunks have left the party?
        for (auto id : meshedChunks)
            PendingMeshUnloads.insert(id);
    }

    ChunkId id;
    while (Builder.PopChunk(&id))
    {
        ValidateChunkMesh(id);
    }

    double gpuTimeLimit = 1.0/60.0;

    double startTime = GetTime();
    while (GetTime() - startTime < gpuTimeLimit)
    {
        if (Mesher.PopChunk(&id))
        {
            auto* chunk = Map.GetChunk(id);
            chunk->SetStatus(ChunkStatus::Useable);
            UploadMesh(&chunk->ChunkMesh, false);
            ChunksWithMeshes.insert(id.Id);
        }
        else if (!PendingMeshUnloads.empty())
        {
            uint64_t rawId = *PendingMeshUnloads.begin();
            PendingMeshUnloads.erase(rawId);

            auto* chunk = Map.GetChunk(ChunkId(rawId));
            if (chunk)
            {
                chunk->Alpha = 0;
                UnloadMesh(chunk->ChunkMesh);
                chunk->ChunkMesh.vaoId = 0;
                chunk->SetStatus(ChunkStatus::Generated);
                ChunksWithMeshes.erase(rawId);
            }
        }
        else
        {
            break;
        }
    }
}

void ChunkManager::Abort()
{
    Builder.Abort();
    Mesher.Abort();

    for (auto& rawId : ChunksWithMeshes)
    {
        auto* chunk = Map.GetChunk(ChunkId(rawId));
        if (chunk)
        {
            chunk->Alpha = 0;
            UnloadMesh(chunk->ChunkMesh);
            chunk->ChunkMesh.vaoId = 0;
            chunk->SetStatus(ChunkStatus::Generated);
        }
    }

    ChunksWithMeshes.clear();
}

void ChunkManager::ValidateChunkGeneration(Voxels::ChunkId id)
{
    auto* chunk = Map.GetChunk(id);

    if (!chunk || chunk->GetStatus() == ChunkStatus::Empty)
    {
        Builder.PushChunk(id);
    }
}

void ChunkManager::ValidateChunkMesh(Voxels::ChunkId id)
{
    auto* chunk = Map.GetChunk(id);

    if (!chunk || chunk->GetStatus() == ChunkStatus::Empty)
    {
        Builder.PushChunk(id);
    }
    else if (chunk->GetStatus() < ChunkStatus::Meshing)
    {
        chunk->SetStatus(ChunkStatus::Meshing);
        Mesher.PushChunk(id);
    }
}
void ChunkManager::DoForEachRenderChunk(std::function<void(Voxels::Chunk*)> func)
{
    Chunk* chunk = Map.GetChunk(CurrentChunk);
    if (chunk && chunk->GetStatus() == ChunkStatus::Useable)
        func(chunk);

    for (auto& area : RenderArea)
    {
        area.DoForEach([this, func](ChunkId id) 
            {
                Chunk* chunk = Map.GetChunk(id);
                if (chunk && chunk->GetStatus() == ChunkStatus::Useable)
                    func(chunk);
            });
    }
}

