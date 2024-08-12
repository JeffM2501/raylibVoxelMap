#include "world_def.h"

#include "external/stb_perlin.h"

using namespace Voxels;

void SetupWorldData(Texture2D& texture)
{
    float blockWidth = 1.0f / 8.0f;
    float blockHeight = 1;

    SetBlockInfo(Air, Rectangle{ 0,0,0,0 }, false);
    SetBlockInfo(Grass, Rectangle{ blockWidth * 2,0,blockWidth * 3,blockHeight }, Rectangle{ 0,0,blockWidth,blockHeight }, Rectangle{ blockWidth,0,blockWidth * 2,blockHeight });
    SetBlockInfo(Dirt, Rectangle{ blockWidth,0,blockWidth * 2,blockHeight });
    SetBlockInfo(Stone, Rectangle{ blockWidth * 3,0,blockWidth * 4,blockHeight });
    SetBlockInfo(Bedrock, Rectangle{ blockWidth * 4,0,blockWidth * 5,blockHeight });

    constexpr int chunkCount = 5;
}

void ChunkGenerationFunction(Voxels::Chunk& chunk)
{
    int32_t chunkH = chunk.Id.Coordinate.h;
    int32_t chunkV = chunk.Id.Coordinate.v;

    for (int v = 0; v < Chunk::ChunkSize; v++)
    {
        for (int h = 0; h < Chunk::ChunkSize; h++)
        {
            float hvScale = 0.02f;

            int64_t worldH = (h + (chunkH * Chunk::ChunkSize));
            int64_t worldV = (v + (chunkV * Chunk::ChunkSize));

            int depthLimit = 8 + int((stb_perlin_fbm_noise3(worldH * hvScale, worldV * hvScale, 1.0f, 2.0f, 0.5f, 6) + 1) * 0.5f * 16);

            for (int d = 0; d < depthLimit; d++)
            {
                if (d == 0)
                    chunk.SetVoxel(h, v, d, Bedrock);
                else if (d < 4)
                    chunk.SetVoxel(h, v, d, Stone);
                else if (d == depthLimit - 1)
                    chunk.SetVoxel(h, v, d, Grass);
                else
                    chunk.SetVoxel(h, v, d, Dirt);
            }
        }
    }

    chunk.SetStatus(ChunkStatus::Generated);
}
