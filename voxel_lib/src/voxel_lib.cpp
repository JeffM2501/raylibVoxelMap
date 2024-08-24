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

    BlockType Chunk::GetVoxel(int h, int v, int d)
    {
        if (h < 0 || h >= ChunkSize || v < 0 || v >= ChunkSize || d < 0 || d >= ChunkHeight)
            return InvalidBlock;

        return Blocks[(d * ChunkSize * ChunkSize) + (v * ChunkSize) + h];
    }

    int Chunk::GetTopBlockDepth(int h, int v)
    {
        for (int d = ChunkHeight; d > 0; d--)
        {
            if (BlockIsSolid(h, v, d-1))
                return d-1;
        }

        return -1;
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
            return true;

        return BlockInfos[block].Solid;
    }

    Voxels::ChunkStatus Chunk::GetStatus() const
    {
        std::lock_guard<std::mutex> lock(StatusLock);
        return Status;
    }

    void Chunk::SetStatus(ChunkStatus status)
    {
        std::lock_guard<std::mutex> lock(StatusLock);
        Status = status;
    }

    ChunkVisibilityRequirement Chunk::GetVisRequirement() const
    {
        std::lock_guard<std::mutex> lock(StatusLock);
        return VisStatus;
    }

    void Chunk::SetVisRequirement(ChunkVisibilityRequirement status)
    {
        std::lock_guard<std::mutex> lock(StatusLock);
        VisStatus = status;
    }

    Chunk& World::AddChunk(int32_t h, int32_t v)
    {
        ChunkId id;
        id.Coordinate.h = h;
        id.Coordinate.v = v;

        std::lock_guard <std::mutex> lock(ChunkLock);
        // todo, get the ref on insert
        Chunks.try_emplace(id.Id).second;
        Chunks[id.Id].Id = id;
        return Chunks[id.Id];
    }

    Voxels::Chunk* World::GetChunk(int32_t h, int32_t v)
    {
        ChunkId id;
        id.Coordinate.h = h;
        id.Coordinate.v = v;
        return GetChunk(id);
    }

    Voxels::Chunk* World::GetChunk(ChunkId id)
    {
        std::lock_guard <std::mutex> lock(ChunkLock);

        auto itr = Chunks.find(id.Id);
        if (itr == Chunks.end())
            return nullptr;

        return &(itr->second);
    }

    bool World::SurroundingChunksGenerated(ChunkId id) const
    {
        std::lock_guard <std::mutex> lock(ChunkLock);

        for (int h = -1; h <= 1; h++)
        {
            for (int v = -1; v <= 1; v++)
            {
                if (h == 0 && v == 0)
                    continue;

                ChunkId siblingChunk;
                siblingChunk.Coordinate.h = id.Coordinate.h + h;
                siblingChunk.Coordinate.v = id.Coordinate.v + v;

                auto itr = Chunks.find(siblingChunk.Id);
                if (itr == Chunks.end() || itr->second.GetStatus() == ChunkStatus::Empty)
                    return false;
            }
        }
        return true;
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

        Chunk* chunkData = GetChunk(chunk);
        if (!chunkData)
            return InvalidBlock;

        return chunkData->GetVoxel(h, v, d);
    }

    bool World::BlockIsSolid(ChunkId chunk, int h, int v, int d)
    {
        BlockType block = GetVoxel(chunk, h, v, d);
        if (block == InvalidBlock)
            return true;

        return BlockInfos[block].Solid;
    }
}