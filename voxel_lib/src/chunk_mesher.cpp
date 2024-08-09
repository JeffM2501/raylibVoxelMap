#include "chunk_mesher.h"


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
}