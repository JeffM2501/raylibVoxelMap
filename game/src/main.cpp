/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

For a C++ project simply rename the file to .cpp and run premake

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

#include "raylib.h"

#include "resource_dir.h"
#include "lighting_system.h"

#include "object_transform.h"
#include "raymath_operators.h"


#include "voxel_lib.h"
#include "chunk_mesher.h"
#include "world_builder.h"

#include "world_def.h"
#include "external/stb_perlin.h"

#include <set>

using namespace Voxels;

Texture2D BlockTexture = { 0 };

World   Map;

std::vector<ChunkId> UseableChunks;
std::vector<ChunkId> GeneratedChunks;

std::set<uint64_t> RequestedChunks;

WorldBuilder Builder(Map, ChunkGenerationFunction);
ChunkMeshTaskPool Mesher(Map);

void SetupBlocks()
{
    BlockTexture = LoadTexture("blockmap.png");
    GenTextureMipmaps(&BlockTexture);
    SetTextureFilter(BlockTexture, TEXTURE_FILTER_ANISOTROPIC_16X);

    SetupWorldData(BlockTexture);

    constexpr int chunkCount = 8;

    ChunkId id;
    for (id.Coordinate.h = -chunkCount; id.Coordinate.h <= chunkCount; id.Coordinate.h++)
    {
        for (id.Coordinate.v = -chunkCount; id.Coordinate.v <= chunkCount; id.Coordinate.v++)
        {
            RequestedChunks.insert(id.Id);
            Builder.PushChunk(id);
        }
    }
}

void MeshChunk()
{
    ChunkId id;
    while (Builder.PopChunk(&id))
    {
        auto itr = RequestedChunks.find(id.Id);
        if (itr != RequestedChunks.end())
            RequestedChunks.erase(itr);

        GeneratedChunks.push_back(id);
        Mesher.PushChunk(id);
    }

    int loadLimit = 3;

    for (int i = 0; i < loadLimit; i++)
    {
        if (!Mesher.PopChunk(&id))
            return;

        Chunk* chunk = Map.GetChunk(id);

        if (!chunk)
            continue;

        UploadMesh(&chunk->ChunkMesh, false);
        chunk->SetStatus(ChunkStatus::Useable);

        UseableChunks.push_back(id);
    }
}

void MoveCamera(ObjectTransform& transform)
{
    double startTime = GetTime();
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        constexpr float mouseFactor = 0.1f;
        transform.RotateY(GetMouseDelta().x * -mouseFactor);
        transform.RotateH(GetMouseDelta().y * -mouseFactor);
    }

    float speed = 10 * GetFrameTime();
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
        speed *= 5;

    if (IsKeyDown(KEY_W))
        transform.MoveD(speed);
    if (IsKeyDown(KEY_S))
        transform.MoveD(-speed);
    if (IsKeyDown(KEY_A))
        transform.MoveH(speed);
    if (IsKeyDown(KEY_D))
        transform.MoveH(-speed);
    if (IsKeyDown(KEY_E))
        transform.MoveV(speed);
    if (IsKeyDown(KEY_Q))
        transform.MoveV(-speed);
}

void UnloadMeshes()
{
    for (auto id : UseableChunks)
    {
        Chunk* chunk = Map.GetChunk(id);
        if (!chunk || chunk->GetStatus() != ChunkStatus::Useable)
            continue;

        UnloadMesh(chunk->ChunkMesh);
        chunk->ChunkMesh.vaoId = 0;
        chunk->SetStatus(ChunkStatus::Generated);
    }
}

int main()
{
    SearchAndSetResourceDir("resources");

    // set up the window
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 800, "Voxels");
    SetTargetFPS(500);

    SetupBlocks();

    auto shader = LoadShader("shaders/lighting.vert", "shaders/lighting.frag");

    auto fogFactorLoc = GetShaderLocation(shader, "fogDensity");
    auto fogColorLoc = GetShaderLocation(shader, "fogColor");

    float factor = 0.005f;
    SetShaderValue(shader, fogFactorLoc, &factor, SHADER_UNIFORM_FLOAT);

    float fogColor[4] = { SKYBLUE.r / 255.0f,SKYBLUE.g / 255.0f,SKYBLUE.b / 255.0f, 0};
    SetShaderValue(shader, fogColorLoc, fogColor, SHADER_UNIFORM_VEC4);

    Lights::SetLightingShader(shader);

    Camera3D ViewCamera = { 0 };
    ViewCamera.fovy = 45;
    ObjectTransform CameraTransform(false);

    CameraTransform.SetPosition(0, Chunk::ChunkHeight * 0.45f, -10);
    CameraTransform.RotateY(-45);
    CameraTransform.SetCamera(ViewCamera);

    Material cubeMat = LoadMaterialDefault();
    cubeMat.shader = Lights::GetLightingShader();
    cubeMat.maps[MATERIAL_MAP_ALBEDO].texture = BlockTexture;

    Vector3 lightDirection = { -2, -3, 1 };
    auto* light = static_cast<Lights::DirectionalLight*>(Lights::AddLight(Lights::LightTypes::Directional));
    light->SetDirection(lightDirection);

    // game loop
    while (!WindowShouldClose())
    {
        MoveCamera(CameraTransform);

        ChunkId currentChunk;
        currentChunk.Coordinate.h = CameraTransform.GetPosition().x / float(Chunk::ChunkSize);
        currentChunk.Coordinate.v = CameraTransform.GetPosition().z / float(Chunk::ChunkSize);

        auto* thisChunk = Map.GetChunk(currentChunk);
        if (thisChunk == nullptr)
        {
            if (RequestedChunks.find(currentChunk.Id) == RequestedChunks.end())
            {
                ChunkId newChunk = currentChunk;

                for (newChunk.Coordinate.h = currentChunk.Coordinate.h-1; newChunk.Coordinate.h <= currentChunk.Coordinate.h+1; newChunk.Coordinate.h++)
                {
                    for (newChunk.Coordinate.v = currentChunk.Coordinate.v - 1; newChunk.Coordinate.v <= currentChunk.Coordinate.v+1; newChunk.Coordinate.v++)
                    {
                        if (RequestedChunks.find(newChunk.Id) == RequestedChunks.end() && Map.GetChunk(newChunk) == nullptr)
                        {
                            RequestedChunks.insert(newChunk.Id);
                            Builder.PushChunk(newChunk);
                        }
                    }
                }
                
            }
        }

        MeshChunk();    

        // drawing
        BeginDrawing();
        ClearBackground(SKYBLUE);

        CameraTransform.SetCamera(ViewCamera);
        Lights::UpdateLights(ViewCamera);
        BeginMode3D(ViewCamera);

        for (auto id : GeneratedChunks)
        {
            Chunk* chunk = Map.GetChunk(id);
            if (!chunk)
                continue;

            constexpr float fadeSpeed = 1.0f/ 0.5f;

            if (chunk->GetStatus() == ChunkStatus::Useable)
            {
                float color[4] = { 1,1,1,1 };
                if (chunk->Alpha < 1)
                {
                    chunk->Alpha += GetFrameTime() * fadeSpeed;
                    if (chunk->Alpha > 1)
                        chunk->Alpha = 1;
                }

                cubeMat.maps[MATERIAL_MAP_DIFFUSE].color.a = chunk->Alpha * 255;

                DrawMesh(chunk->ChunkMesh, cubeMat, MatrixTranslate(id.Coordinate.h * float(Chunk::ChunkSize), 0, id.Coordinate.v * float(Chunk::ChunkSize)));
            }
            else
            {
                Vector3 center = { id.Coordinate.h * float(Chunk::ChunkSize) + float(Chunk::ChunkSize) * 0.5f,
                                   float(Chunk::ChunkHeight) * 0.5f,
                                   id.Coordinate.v * float(Chunk::ChunkSize) + float(Chunk::ChunkSize) * 0.5f };

                DrawCubeWires(center, Chunk::ChunkSize-1, Chunk::ChunkHeight, Chunk::ChunkSize-1,RED);

            }
        }

         rlDrawRenderBatchActive();
         rlDisableDepthTest();
         DrawLine3D(Vector3{ 0,0,0 }, Vector3{ 10,0,0 }, RED);
         DrawLine3D(Vector3{ 0,0,0 }, Vector3{ 0,5,0 }, GREEN);
         DrawLine3D(Vector3{ 0,0,0 }, Vector3{ 0,0,10 }, BLUE);
         rlDrawRenderBatchActive();
         rlEnableDepthTest();

        EndMode3D();
        DrawFPS(0, 0);
        EndDrawing();
    }

    Mesher.Abort();
    Builder.Abort();

    UnloadMeshes();

    UnloadTexture(BlockTexture);

    // cleanup
    CloseWindow();
    return 0;
}