#pragma once

#include "voxel_lib.h"
#include "raymath.h"

static constexpr Voxels::BlockType Air = 0;
static constexpr Voxels::BlockType Grass = 1;
static constexpr Voxels::BlockType Dirt = 2;
static constexpr Voxels::BlockType Stone = 3;
static constexpr Voxels::BlockType Bedrock = 4;
static constexpr Voxels::BlockType Gold = 5;
static constexpr Voxels::BlockType Copper = 6;
//static constexpr Voxels::BlockType Open = 7;
static constexpr Voxels::BlockType Tree = 8;
static constexpr Voxels::BlockType Leaves = 9;

void SetupWorldData(Texture2D& texture);

void ChunkGenerationFunction(Voxels::Chunk& chunk);
void ChunkPopulationFunction(Voxels::Chunk& chunk);
