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


#include "external/stb_perlin.h"

using namespace Voxels;

constexpr BlockType Air = 0;
constexpr BlockType Grass = 1;
constexpr BlockType Dirt = 2;
constexpr BlockType Stone = 3;
constexpr BlockType Bedrock = 4;

Texture2D BlockTexture = { 0 };

World   Map;

std::vector<ChunkId> UseableChunks;

void ChunkGenerationFunction(Chunk& chunk)
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

WorldBuilder Builder(Map, ChunkGenerationFunction);
ChunkMeshTaskPool Mesher(Map);

void SetupBlocks()
{
    BlockTexture = LoadTexture("blockmap.png");
    GenTextureMipmaps(&BlockTexture);
    SetTextureFilter(BlockTexture, TEXTURE_FILTER_ANISOTROPIC_16X);

    float blockWidth = 1.0f/8.0f;
    float blockHeight = 1;

    SetBlockInfo(Air, Rectangle{ 0,0,0,0 }, false);
    SetBlockInfo(Grass, Rectangle{ blockWidth * 2,0,blockWidth * 3,blockHeight }, Rectangle{ 0,0,blockWidth,blockHeight }, Rectangle{ blockWidth,0,blockWidth*2,blockHeight });
    SetBlockInfo(Dirt, Rectangle{ blockWidth,0,blockWidth*2,blockHeight });
    SetBlockInfo(Stone, Rectangle{ blockWidth * 3,0,blockWidth * 4,blockHeight });
    SetBlockInfo(Bedrock, Rectangle{ blockWidth * 4,0,blockWidth * 5,blockHeight });

    constexpr int chunkCount = 5;

    ChunkId id;
    for (id.Coordinate.h = -chunkCount; id.Coordinate.h < chunkCount; id.Coordinate.h++)
    {
        for (id.Coordinate.v = -chunkCount; id.Coordinate.v < chunkCount; id.Coordinate.v++)
        {
            Builder.PushChunk(id);
        }
    }
}

void MeshChunk()
{
    ChunkId id;
    while (Builder.PopChunk(&id))
    {
        Mesher.PushChunk(id);
    }

    if (!Mesher.PopChunk(&id))
        return;

    Chunk* chunk = Map.GetChunk(id);

    if (!chunk)
        return;

    UploadMesh(&chunk->ChunkMesh, false);
    chunk->SetStatus(ChunkStatus::Useable);

    UseableChunks.push_back(id);
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

int main()
{
    SearchAndSetResourceDir("resources");

    // set up the window
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 800, "Voxels");
    SetTargetFPS(500);

    SetupBlocks();

    Lights::SetLightingShader(LoadShader("shaders/lighting.vert", "shaders/lighting.frag"));

    Camera3D ViewCamera = { 0 };
    ViewCamera.fovy = 45;
    ObjectTransform CameraTransform(false);

    CameraTransform.SetPosition(0, Chunk::ChunkHeight * 0.25f, -10);
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
        MeshChunk();

        MoveCamera(CameraTransform);

        // drawing
        BeginDrawing();
        ClearBackground(DARKGRAY);

        CameraTransform.SetCamera(ViewCamera);
        Lights::UpdateLights(ViewCamera);
        BeginMode3D(ViewCamera);

        for (auto id : UseableChunks)
        {
            Chunk* chunk = Map.GetChunk(id);
            if (!chunk || chunk->GetStatus() != ChunkStatus::Useable)
                continue;

            DrawMesh(chunk->ChunkMesh, cubeMat, MatrixTranslate(id.Coordinate.h * float(Chunk::ChunkSize), 0, id.Coordinate.v * float(Chunk::ChunkSize)));
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

    // cleanup
    CloseWindow();
    return 0;
}