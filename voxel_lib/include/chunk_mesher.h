#pragma once

#include "geometry_builder.h"
#include "voxel_lib.h"

#include <mutex>

namespace Voxels
{
    class ChunkMesher
    {
    public:
        enum class Status
        {
            Unbuilt,
            Building,
            Built,
        };

        ChunkMesher(World& world, ChunkId chunk);

        void BuildMesh();

        Mesh GetMesh();

        Status GetStatus() const;

    private:
        World&              Map;
        ChunkId             MapChunk;
        CubeGeometryBuilder Builder;
        Mesh                ChunkMesh = { 0 };
        Status              BuildStatus = Status::Unbuilt;

        mutable std::mutex  StatusLock;

        void SetStatus(Status status);

        int GetChunkFaceCount();
    };
}