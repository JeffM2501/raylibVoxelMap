#include "voxel_lib.h"

namespace Voxels
{
    std::unordered_map<BlockType, BlockInfo> BlockInfos;

    void SetBlockInfo(BlockType blockId, Rectangle& blockSides, bool solid)
    {
        BlockInfo block;
        for (int i = 0; i < 6; i++)
            block.FaceUVs[i] = blockSides;
        block.BlockID = blockId;
        block.Solid = solid;

        BlockInfos.insert_or_assign(blockId, block);
    }

    void SetBlockInfo(BlockType blockId, Rectangle& blockSides, Rectangle& blockTop, bool solid)
    {
        BlockInfo block;
        for (int i = 0; i < 6; i++)
            block.FaceUVs[i] = blockSides;

        block.FaceUVs[UpFace] = blockTop;
        block.BlockID = blockId;
        block.Solid = solid;

        BlockInfos.insert_or_assign(blockId, block);
    }

    void SetBlockInfo(BlockType blockId, Rectangle& blockSides, Rectangle& blockTop, Rectangle& blockBottom, bool solid)
    {
        BlockInfo block;
        for (int i = 0; i < 6; i++)
            block.FaceUVs[i] = blockSides;

        block.FaceUVs[UpFace] = blockTop;
        block.FaceUVs[DownFace] = blockBottom;
        block.BlockID = blockId;
        block.Solid = solid;

        BlockInfos.insert_or_assign(blockId, block);
    }

    Voxels::BlockType Chunk::GetVoxel(int h, int v, int d)
    {
        if (h < 0 || h >= ChunkSize || v < 0 || v >= ChunkSize || d < 0 || d >= ChunkHeight)
            return InvalidBlock;

        return Blocks[(d * ChunkSize * ChunkSize) + (v * ChunkSize) + h];
    }

    void Chunk::SetVoxel(int h, int v, int d, BlockType block)
    {
        if (h < 0 || h >= ChunkSize || v < 0 || v >= ChunkSize || d < 0 || d >= ChunkHeight)
            return;

        Blocks[(d * ChunkSize * ChunkSize) + (v * ChunkSize) + h] = block;
    }

    bool Chunk::BlockIsSolid(int h, int v, int d)
    {
        BlockType block = GetVoxel(h, v, d);
        if (block == InvalidBlock)
            return false;

        return BlockInfos[block].Solid;
    }

    Chunk& World::AddChunk(int32_t h, int32_t v)
    {
        ChunkId id;
        id.Coordinate.h = h;
        id.Coordinate.v = v;

        // todo, get the ref on insert
        Chunks.try_emplace(id.Id).second;
        Chunks[id.Id].Id = id;
        return Chunks[id.Id];
    }

    BlockType World::GetVoxel(ChunkId chunk, int h, int v, int d)
    {
        if (d < 0 || d >= Chunk::ChunkHeight)
            return InvalidBlock;

        while (h < 0)
        {
            chunk.Coordinate.h -= 1;
            h += Chunk::ChunkSize;
        }

        while (h >= Chunk::ChunkSize)
        {
            chunk.Coordinate.h += 1;
            h -= Chunk::ChunkSize;
        }

        while (v < 0)
        {
            chunk.Coordinate.v -= 1;
            v += Chunk::ChunkSize;
        }

        while (v >= Chunk::ChunkSize)
        {
            chunk.Coordinate.v += 1;
            v -= Chunk::ChunkSize;
        }

        auto itr = Chunks.find(chunk.Id);
        if (itr == Chunks.end())
            return InvalidBlock;

        return itr->second.GetVoxel(h, v, d);

    }

    bool World::BlockIsSolid(ChunkId chunk, int h, int v, int d)
    {
        BlockType block = GetVoxel(chunk, h, v, d);
        if (block == InvalidBlock)
            return false;

        return BlockInfos[block].Solid;
    }
}