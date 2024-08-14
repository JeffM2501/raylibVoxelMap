// C library
/*
-- Copyright (c) 2020-2024 Jeffery Myers
--
--This software is provided "as-is", without any express or implied warranty. In no event 
--will the authors be held liable for any damages arising from the use of this software.

--Permission is granted to anyone to use this software for any purpose, including commercial 
--applications, and to alter it and redistribute it freely, subject to the following restrictions:

--  1. The origin of this software must not be misrepresented; you must not claim that you 
--  wrote the original software. If you use this software in a product, an acknowledgment 
--  in the product documentation would be appreciated but is not required.
--
--  2. Altered source versions must be plainly marked as such, and must not be misrepresented
--  as being the original software.
--
--  3. This notice may not be removed or altered from any source distribution.
*/

#pragma once

#include <stdint.h>
#include <unordered_map>
#include <mutex>
#include <functional>

#include "raylib.h"

namespace Voxels
{
    using BlockType = uint8_t;
    static constexpr BlockType InvalidBlock = BlockType(-1);

    // indexes for the 6 faces of a cube
    static constexpr int SouthFace = 0;
    static constexpr int NorthFace = 1;
    static constexpr int WestFace = 2;
    static constexpr int EastFace = 3;
    static constexpr int UpFace = 4;
    static constexpr int DownFace = 5;

    struct BlockInfo
    {
        Rectangle FaceUVs[6] = { 0 };
        BlockType BlockID = InvalidBlock;
        bool Solid = true;
    };

    extern std::unordered_map<BlockType, BlockInfo> BlockInfos;

    void SetBlockInfo(BlockType blockId, Rectangle& blockSides, bool solid = true);
    void SetBlockInfo(BlockType blockId, Rectangle& blockSides, Rectangle& blockTop, bool solid = true);
    void SetBlockInfo(BlockType blockId, Rectangle& blockSides, Rectangle& blockTop, Rectangle& blockBottom, bool solid = true);

    struct ChunkCoordinate
    {
        int32_t h;
        int32_t v;
    };

    union ChunkId
    {
        ChunkCoordinate Coordinate;
        uint64_t Id = uint64_t (-1);

        ChunkId() {}

        ChunkId(int32_t h, int32_t v)
        {
            Coordinate.h = h;
            Coordinate.v = v;
        }

        ChunkId(uint64_t id)
        {
            Id = id;
        }

        bool IsValid() const
        {
            return Id != -1;
        }
    };

    enum class ChunkStatus
    {
        Empty,
        Generating,
        Generated,
        Meshing,
        Meshed,
        Useable,
    };

    enum class ChunkVisibilityRequirement
    {
        Unknown,
        NeedLoad,
        NeedMesh,
        NotNeeded,
    };

    class Chunk
    {
    public:
        static constexpr int ChunkSize = 16;
        static constexpr int ChunkHeight = 32;

        ChunkId Id;

        BlockType GetVoxel(int h, int v, int d);
        void SetVoxel(int h, int v, int d, BlockType block);

        bool BlockIsSolid(int h, int v, int d);

        Mesh ChunkMesh;

        ChunkStatus GetStatus() const;
        void SetStatus(ChunkStatus status);

        ChunkVisibilityRequirement GetVisRequirement() const;
        void SetVisRequirement(ChunkVisibilityRequirement status);

        float Alpha = 0;

    private:
        BlockType Blocks[ChunkSize * ChunkSize * ChunkHeight] = { 0 };

        mutable std::mutex StatusLock;
        ChunkStatus Status = ChunkStatus::Empty;
        ChunkVisibilityRequirement VisStatus = ChunkVisibilityRequirement::Unknown;
    };

    class World
    {
    public:
        Chunk& AddChunk(int32_t h, int32_t v);

        Chunk* GetChunk(int32_t h, int32_t v);
        Chunk* GetChunk(ChunkId id);

        bool SurroundingChunksGenerated(ChunkId id) const;

        BlockType GetVoxel(ChunkId chunk, int h, int v, int d);
        bool BlockIsSolid(ChunkId chunk, int h, int v, int d);

    private:
        mutable std::mutex ChunkLock;
        std::unordered_map<uint64_t, Chunk> Chunks;

    };
}
