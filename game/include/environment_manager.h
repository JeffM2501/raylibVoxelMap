#pragma once

struct Camera3D;
namespace Environment
{
    void Load();
    void DrawBackground(const Camera3D& camera);
    void DrawForeground(const Camera3D& camera);
    void DrawPreChunk(const Camera3D& camera);
    void DrawPostChunk(const Camera3D& camera);
    void Unload();
}