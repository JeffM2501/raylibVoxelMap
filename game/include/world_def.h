#pragma once

#include "voxel_lib.h"
#include "raymath.h"

static constexpr Voxels::BlockType Air = 0;
static constexpr Voxels::BlockType Grass = 1;
static constexpr Voxels::BlockType Dirt = 2;
static constexpr Voxels::BlockType Stone = 3;
static constexpr Voxels::BlockType Bedrock = 4;

void SetupWorldData(Texture2D& texture);

void ChunkGenerationFunction(Voxels::Chunk& chunk);
