#include "environment_manager.h"

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <stdint.h>

namespace Environment
{

    Color HorizonColor[2] = { WHITE, Color{ 0, 41, 86, 255 } };
    Color ZenithColor[2] = { SKYBLUE, BLACK };
    Color UndergroundColor[2] = { DARKBLUE, DARKBROWN };

    float domeRadius = 100.0f;
    Shader skyShader;
    Mesh Skydome = { 0 };
    Material DomeMaterial;

    void ColorToFloat(Color color, float* colorF)
    {
        colorF[0] = color.r / 255.0f;
        colorF[1] = color.g / 255.0f;
        colorF[2] = color.b / 255.0f;
        colorF[3] = color.a / 255.0f;
    }

    void Load()
    {
        skyShader = LoadShader("shaders/lighting.vert", "shaders/skydome.frag");

        int todIndex = 0;

        float zenith[4];
        ColorToFloat(ZenithColor[todIndex], zenith);
        SetShaderValue(skyShader, GetShaderLocation(skyShader, "zenithColor"), zenith, SHADER_UNIFORM_VEC4);

        float horizon[4];
        ColorToFloat(HorizonColor[todIndex], horizon);
        SetShaderValue(skyShader, GetShaderLocation(skyShader, "horizonColor"), horizon, SHADER_UNIFORM_VEC4);

        float underground[4];
        ColorToFloat(UndergroundColor[todIndex], underground);
        SetShaderValue(skyShader, GetShaderLocation(skyShader, "UndergroundColor"), underground, SHADER_UNIFORM_VEC4);

        SetShaderValue(skyShader, GetShaderLocation(skyShader, "radius"), &domeRadius, SHADER_UNIFORM_FLOAT);

        Skydome = GenMeshSphere(-domeRadius, 16,16);

        DomeMaterial = LoadMaterialDefault();
        DomeMaterial.shader = skyShader;
    }

    void DrawBackground(const Camera3D& camera)
    {
       
    }

    void DrawForeground(const Camera3D& camera)
    {
          
    }

    void DrawPreChunk(const Camera3D& camera)
    {
        rlDrawRenderBatchActive();
        rlDisableDepthMask();

        DrawMesh(Skydome, DomeMaterial, MatrixTranslate(camera.position.x, camera.position.y, camera.position.z));


        rlPushMatrix();
        rlTranslatef(camera.position.x, camera.position.y, camera.position.z);

        float sunspeed = 1.0f / 5.0f;

        rlRotatef((float(GetTime()) * sunspeed) + 45, 0, 0, 1);
        rlTranslatef(50, 0, 0);

        float coronaSize = 6 + sinf(float(GetTime()));
        DrawCube(Vector3{ 0,0,0 }, -coronaSize,-coronaSize,-coronaSize, ColorAlpha(YELLOW,0.5f));
        DrawCube(Vector3{ 0,0,0 }, 4,4,4, ColorAlpha(RAYWHITE, 0.75f));

        rlPopMatrix();

        rlDrawRenderBatchActive();
        rlEnableDepthMask();
    }

    void DrawPostChunk(const Camera3D& camera)
    {

    }

    void Unload()
    {
        UnloadMesh(Skydome);
        UnloadShader(skyShader);
    }
}