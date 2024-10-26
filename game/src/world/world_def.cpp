#include "world_def.h"

#include "external/stb_perlin.h"

using namespace Voxels;

void SetupWorldData(Texture2D& texture)
{
    float blockWidth = 1.0f / 8.0f;
    float blockHeight = 1.0f/2.0f;

    SetBlockInfo(Air, Rectangle{ 0,0,0,0 }, false);
    SetBlockInfo(Grass, Rectangle{ blockWidth * 2,0,blockWidth * 3,blockHeight }, Rectangle{ 0,0,blockWidth,blockHeight }, Rectangle{ blockWidth,0,blockWidth * 2,blockHeight });
    SetBlockInfo(Dirt, Rectangle{ blockWidth,0,blockWidth * 2,blockHeight });
    SetBlockInfo(Stone, Rectangle{ blockWidth * 3,0,blockWidth * 4,blockHeight });
    SetBlockInfo(Bedrock, Rectangle{ blockWidth * 4,0,blockWidth * 5,blockHeight });

    SetBlockInfo(Gold, Rectangle{ blockWidth * 5,0,blockWidth * 6,blockHeight });
    SetBlockInfo(Copper, Rectangle{ blockWidth * 6,0,blockWidth * 7,blockHeight });

    SetBlockInfo(Tree, Rectangle{ 0, blockHeight, blockWidth * 1, blockHeight*2 }, Rectangle{ blockWidth * 1, blockHeight,blockWidth * 2, blockHeight * 2 });
    SetBlockInfo(Leaves, Rectangle{ blockWidth * 2, blockHeight,blockWidth * 3, blockHeight*2 });

    constexpr int chunkCount = 5;
}

void ChunkPopulationFunction(Voxels::Chunk& chunk)
{
    int32_t chunkH = chunk.Id.Coordinate.h;
    int32_t chunkV = chunk.Id.Coordinate.v;

    for (int v = 0; v < Chunk::ChunkSize; v++)
    {
        for (int h = 0; h < Chunk::ChunkSize; h++)
        {
            float hvScale = 4.5f;

            int64_t worldH = (h + (chunkH * Chunk::ChunkSize));
            int64_t worldV = (v + (chunkV * Chunk::ChunkSize));

            if (stb_perlin_fbm_noise3(worldH * hvScale, worldV * hvScale, 1.0f, 3.0f, 1.5f, 2) > 1.3f)
            {
                int d = chunk.GetTopBlockDepth(h, v);
                if (chunk.GetVoxel(h, v, d) == Grass)
                {
                    for (int treeD = 0; treeD < 4; treeD++)
                    {
                        chunk.SetVoxel(h, v, d + 1 + treeD, Tree);
                    }
                    chunk.SetVoxel(h, v, d + 1 + 4, Leaves);
                }
            }
        }
    }
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

            float limit = 0.5f;

            float holeFactor = stb_perlin_fbm_noise3(worldH * hvScale, worldV * hvScale, 50.0f, 2.0f, 0.5f, 6);
            if (holeFactor > limit)
            {
                float holeRange = (holeFactor - limit) / (1.0f - limit);
                holeRange *= 18;

                int min = 10 - int(holeRange / 2);
                int max = 10 + int(holeRange / 2);

                for (int d = min; d <= max; d++)
                {
                    chunk.SetVoxel(h, v, d, Air);
                }
            }

            float oreFactor = stb_perlin_fbm_noise3(worldH * (hvScale), worldV * (hvScale), 20.0f, 2.0f, 0.5f, 6);

            int oreHeight = int(stb_perlin_fbm_noise3(worldH * (hvScale), worldV * (hvScale), 10, 2.0f, 0.5f, 6) * 3);

            limit = 0.8f;
            if (oreFactor > limit)
            {
                int height = 8 + oreHeight * 3;
                if (chunk.GetVoxel(h, v, height) != Air)
                {
                    chunk.SetVoxel(h, v, height, Gold);
                }
            }
            else if(oreFactor < -limit)
            {
                int height = 10 + oreHeight;
                if (chunk.GetVoxel(h, v, height) != Air)
                {
                    chunk.SetVoxel(h, v, height, Copper);
                }
            }

            for (int d = Chunk::ChunkHeight-2; d > 0; d--)
            {
                if (chunk.GetVoxel(h, v, d) == Grass)
                {
                    if (chunk.GetVoxel(h, v, d - 1) == Air)
                    {
                        chunk.SetVoxel(h, v, d, Air);
                        chunk.SetVoxel(h, v, d-1, Grass);
                    }
                }
            }
        }
    }

    chunk.SetStatus(ChunkStatus::Generated);
}
