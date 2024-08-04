#pragma once

#include "raylib.h"

namespace Lights
{
    //----------------------------------------------------------------------------------
    // Defines and Macros
    //----------------------------------------------------------------------------------
    static constexpr int MaxShaderLights = 4; // Max dynamic lights supported by shader


    enum class LightTypes
    {
        Point = 0,
        Directional = 1,
        Spot = 2
    };

    class Light
    {
    public:
        Light(int id);
        virtual ~Light() = default;

        virtual void Update();

        bool IsDirty() const { return Dirty; }

        LightTypes GetType() const { return LightType; }

        void SetPosition(const Vector3& pos);
        void SetIntensity(const Color& color);
        void SetAttenuation(float attenuation);
        void SetFalloff(float falloff);

    protected:
        int ID = 0;

        LightTypes LightType = LightTypes::Directional;

        Vector3 Position = { 0,0,0 };
        Color Intensity = { 255,255,255,255 };

        float Attenuation = 25;
        float Falloff = 50;

        // Shader locations
        int EnabledLoc = -1;
        int TypeLoc = -1;
        int PosLoc = -1;
        int IntensityLoc = -1;

        virtual void OnBindToShader();

        void SetDirty() { Dirty = true; }

    private:
        bool Dirty = false;
    };

    class PointLight : public Light
    {
    public:
        PointLight();
    };

    class DirectionalLight : public Light
    {
    public:
        DirectionalLight();

        void SetDirection(const Vector3& dir);
    protected:
        Vector3 Direction = { 1,-1,1 };
    };

    class SpotLight : public DirectionalLight
    {
    public:
        SpotLight();

        void SetConeAngle(const Vector3& dir);
    protected:
        float Cone = 45;
    };

    void SetLightingShader(Shader shader);

    Light* AddLight(LightTypes lightType);
    void RemoveLight(Light* light);

    void ClearLights(Light* light);

    void UpdateLights(const Camera3D& viewportCamear);
}