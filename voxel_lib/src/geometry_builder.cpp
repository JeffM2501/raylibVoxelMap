#pragma once

#include "geometry_builder.h"
#include "voxel_lib.h"

#include <map>

using namespace Voxels;

// setup the builder with the mesh it is going to fill out
CubeGeometryBuilder::CubeGeometryBuilder(Mesh& mesh) : MeshRef(mesh)
{
}

// we need to know how many triangles are going to be in the mesh before we start
// this way we can allocate the correct buffer sizes for the mesh
void CubeGeometryBuilder::Allocate(int triangles)
{
    // there are 
    MeshRef.vertexCount = triangles * 6;
    MeshRef.triangleCount = triangles * 2;

    MeshRef.vertices = static_cast<float*>(MemAlloc(sizeof(float) * 3 * MeshRef.vertexCount));
    MeshRef.normals = static_cast<float*>(MemAlloc(sizeof(float) * 3 * MeshRef.vertexCount));
    MeshRef.texcoords = static_cast<float*>(MemAlloc(sizeof(float) * 2 * MeshRef.vertexCount));
    MeshRef.colors = nullptr;	// static_cast<unsigned char*>(MemAlloc(sizeof(unsigned char) * 4 * MeshRef.vertexCount));

    MeshRef.animNormals = nullptr;
    MeshRef.animVertices = nullptr;
    MeshRef.boneIds = nullptr;
    MeshRef.boneWeights = nullptr;
    MeshRef.tangents = nullptr;
    MeshRef.indices = nullptr;
    MeshRef.texcoords2 = nullptr;
}

void CubeGeometryBuilder::SetNormal(Vector3& value) 
{ 
    Normal = value;
}

void CubeGeometryBuilder::SetNormal(float x, float y, float z)
{ 
    Normal = Vector3{ x,y,z }; 
}

void CubeGeometryBuilder::SetSetUV(Vector2& value)
{ 
    UV = value;
}

void CubeGeometryBuilder::SetSetUV(float x, float y) 
{ 
    UV = Vector2{ x,y }; 
}

void CubeGeometryBuilder::PushVertex(Vector3& vertex, float xOffset, float yOffset, float zOffset)
{
    size_t index = 0;

    if (MeshRef.colors != nullptr)
    {
        index = TriangleIndex * 12 + VertIndex * 4;
        MeshRef.colors[index] = VertColor.r;
        MeshRef.colors[index + 1] = VertColor.g;
        MeshRef.colors[index + 2] = VertColor.b;
        MeshRef.colors[index + 3] = VertColor.a;
    }

    if (MeshRef.texcoords != nullptr)
    {
        index = TriangleIndex * 6 + VertIndex * 2;
        MeshRef.texcoords[index] = UV.x;
        MeshRef.texcoords[index + 1] = UV.y;
    }

    if (MeshRef.normals != nullptr)
    {
        index = TriangleIndex * 9 + VertIndex * 3;
        MeshRef.normals[index] = Normal.x;
        MeshRef.normals[index + 1] = Normal.y;
        MeshRef.normals[index + 2] = Normal.z;
    }

    index = TriangleIndex * 9 + VertIndex * 3;
    MeshRef.vertices[index] = vertex.x + xOffset;
    MeshRef.vertices[index + 1] = vertex.y + yOffset;
    MeshRef.vertices[index + 2] = vertex.z + zOffset;

    VertIndex++;
    if (VertIndex > 2)
    {
        TriangleIndex++;
        VertIndex = 0;
    }
}

void CubeGeometryBuilder::AddCube(Vector3&& position, bool faces[6], Voxels::BlockType block)
{
    BlockInfo& uvInfo = BlockInfos[block];
    SetSetUV(0, 0);
    //z-
    if (faces[NorthFace])
    {
        Rectangle& uvRect = uvInfo.FaceUVs[NorthFace];

        SetNormal(0, 0, -1);
        SetSetUV(uvRect.x, uvRect.height);
        PushVertex(position);

        SetSetUV(uvRect.width, uvRect.y);
        PushVertex(position, 1, 1, 0);

        SetSetUV(uvRect.width, uvRect.height);
        PushVertex(position, 1, 0, 0);

        SetSetUV(uvRect.x, uvRect.height);
        PushVertex(position);

        SetSetUV(uvRect.x, uvRect.y);
        PushVertex(position, 0, 1, 0);

        SetSetUV(uvRect.width, uvRect.y);
        PushVertex(position, 1, 1, 0);
    }

    // z+
    if (faces[SouthFace])
    {
        Rectangle& uvRect = uvInfo.FaceUVs[SouthFace];

        SetNormal(0, 0, 1);

        SetSetUV(uvRect.x, uvRect.height);
        PushVertex(position, 0, 0, 1);

        SetSetUV(uvRect.width, uvRect.height);
        PushVertex(position, 1, 0, 1);

        SetSetUV(uvRect.width, uvRect.y);
        PushVertex(position, 1, 1, 1);

        SetSetUV(uvRect.x, uvRect.height);
        PushVertex(position, 0, 0, 1);

        SetSetUV(uvRect.width, uvRect.y);
        PushVertex(position, 1, 1, 1);

        SetSetUV(uvRect.x, uvRect.y);
        PushVertex(position, 0, 1, 1);
    }

    // x+
    if (faces[WestFace])
    {
        Rectangle& uvRect = uvInfo.FaceUVs[WestFace];

        SetNormal(1, 0, 0);
        SetSetUV(uvRect.width, uvRect.height);
        PushVertex(position, 1, 0, 1);

        SetSetUV(uvRect.x, uvRect.height);
        PushVertex(position, 1, 0, 0);

        SetSetUV(uvRect.x, uvRect.y);
        PushVertex(position, 1, 1, 0);

        SetSetUV(uvRect.width, uvRect.height);
        PushVertex(position, 1, 0, 1);

        SetSetUV(uvRect.x, uvRect.y);
        PushVertex(position, 1, 1, 0);

        SetSetUV(uvRect.width, uvRect.y);
        PushVertex(position, 1, 1, 1);
    }

    // x-
    if (faces[EastFace])
    {
        Rectangle& uvRect = uvInfo.FaceUVs[EastFace];

        SetNormal(-1, 0, 0);

        SetSetUV(uvRect.width, uvRect.height);
        PushVertex(position, 0, 0, 1);

        SetSetUV(uvRect.x, uvRect.y);
        PushVertex(position, 0, 1, 0);

        SetSetUV(uvRect.x, uvRect.height);
        PushVertex(position, 0, 0, 0);

        SetSetUV(uvRect.width, uvRect.height);
        PushVertex(position, 0, 0, 1);

        SetSetUV(uvRect.width, uvRect.y);
        PushVertex(position, 0, 1, 1);

        SetSetUV(uvRect.x, uvRect.y);
        PushVertex(position, 0, 1, 0);
    }

    if (faces[UpFace])
    {
        Rectangle& uvRect = uvInfo.FaceUVs[UpFace];

        SetNormal(0, 1, 0);

        SetSetUV(uvRect.x, uvRect.y);
        PushVertex(position, 0, 1, 0);

        SetSetUV(uvRect.width, uvRect.height);
        PushVertex(position, 1, 1, 1);

        SetSetUV(uvRect.width, uvRect.y);
        PushVertex(position, 1, 1, 0);

        SetSetUV(uvRect.x, uvRect.y);
        PushVertex(position, 0, 1, 0);

        SetSetUV(uvRect.x, uvRect.height);
        PushVertex(position, 0, 1, 1);

        SetSetUV(uvRect.width, uvRect.height);
        PushVertex(position, 1, 1, 1);
    }

    SetSetUV(0, 0);
    if (faces[DownFace])
    {
        Rectangle& uvRect = uvInfo.FaceUVs[DownFace];

        SetNormal(0, -1, 0);

        SetSetUV(uvRect.x, uvRect.y);
        PushVertex(position, 0, 0, 0);

        SetSetUV(uvRect.width, uvRect.y);
        PushVertex(position, 1, 0, 0);

        SetSetUV(uvRect.width, uvRect.height);
        PushVertex(position, 1, 0, 1);

        SetSetUV(uvRect.x, uvRect.y);
        PushVertex(position, 0, 0, 0);

        SetSetUV(uvRect.width, uvRect.height);
        PushVertex(position, 1, 0, 1);

        SetSetUV(uvRect.x, uvRect.height);
        PushVertex(position, 0, 0, 1);
    }
}
