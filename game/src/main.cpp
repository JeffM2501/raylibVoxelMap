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
#include "environment_manager.h"

#include "object_transform.h"
#include "raymath_operators.h"

#include "voxel_lib.h"
#include "chunk_mesher.h"
#include "world_builder.h"

#include "chunk_manager.h"

#include "world_def.h"
#include "external/stb_perlin.h"

#include "tasks.h"

#include <set>

using namespace Voxels;

Texture2D BlockTexture = { 0 };

World   Map;

ChunkManager Manager(Map);

void SetupBlocks()
{
    BlockTexture = LoadTexture("blockmap.png");
    GenTextureMipmaps(&BlockTexture);
    SetTextureFilter(BlockTexture, TEXTURE_FILTER_ANISOTROPIC_16X);

    SetupWorldData(BlockTexture);

    Manager.Builder.SetTerrainGenerationFunction(ChunkGenerationFunction);
    Manager.Builder.SetPopulateFunction(ChunkPopulationFunction);
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

    SetTraceLogLevel(LOG_WARNING);

    // set up the window
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    int screenX = 1280;
    int screenY = 800;
#ifndef _DEBUG
    screenX = screenY = 0;

    SetConfigFlags(FLAG_FULLSCREEN_MODE);
#endif

    InitWindow(screenX, screenY, "Voxels");
    SetTargetFPS(500);

    Tasks::Init();

    SetupBlocks();
    Environment::Load();

    auto shader = LoadShader("shaders/lighting.vert", "shaders/lighting.frag");

    auto fogFactorLoc = GetShaderLocation(shader, "fogDensity");
    auto fogColorLoc = GetShaderLocation(shader, "fogColor");

    float factor = 100.0f;
    SetShaderValue(shader, fogFactorLoc, &factor, SHADER_UNIFORM_FLOAT);

    float fogColor[4] = { WHITE.r / 255.0f,WHITE.g / 255.0f,WHITE.b / 255.0f, 255};
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

        Manager.Update(CameraTransform.GetPosition());
        // drawing
        BeginDrawing();
        ClearBackground(SKYBLUE);

        CameraTransform.SetCamera(ViewCamera);

        Environment::DrawBackground(ViewCamera);
        Lights::UpdateLights(ViewCamera);

        BeginMode3D(ViewCamera);
        Environment::DrawPreChunk(ViewCamera);
        Manager.DoForEachRenderChunk([&cubeMat](Chunk* chunk)
            {                
                constexpr float fadeSpeed = 1.0f/ 0.5f;

                float color[4] = { 1,1,1,1 };
                if (chunk->Alpha < 1)
                {
                    chunk->Alpha += GetFrameTime() * fadeSpeed;
                    if (chunk->Alpha > 1)
                        chunk->Alpha = 1;
                }

                cubeMat.maps[MATERIAL_MAP_DIFFUSE].color.a = (unsigned char)(chunk->Alpha * 255);

                DrawMesh(chunk->ChunkMesh, cubeMat, MatrixTranslate(chunk->Id.Coordinate.h * float(Chunk::ChunkSize), 0, chunk->Id.Coordinate.v * float(Chunk::ChunkSize)));
            });
        Environment::DrawPostChunk(ViewCamera);
        rlDrawRenderBatchActive();
        rlDisableDepthTest();
        DrawLine3D(Vector3{ 0,0,0 }, Vector3{ 10,0,0 }, RED);
        DrawLine3D(Vector3{ 0,0,0 }, Vector3{ 0,5,0 }, GREEN);
        DrawLine3D(Vector3{ 0,0,0 }, Vector3{ 0,0,10 }, BLUE);
        rlDrawRenderBatchActive();
        rlEnableDepthTest();

        Manager.DrawDebug3D();

        EndMode3D();
        Environment::DrawForeground(ViewCamera);
        DrawFPS(0, 0);
        Manager.DrawDebug2D();

        EndDrawing();
    }
    Tasks::Shutdown();
    Manager.Abort();
    Environment::Unload();

    UnloadTexture(BlockTexture);

    // cleanup
    CloseWindow();
    return 0;
}