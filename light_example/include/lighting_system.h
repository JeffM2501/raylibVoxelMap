#pragma once

#include <string>

struct Shader;
struct Camera3D;
struct Vector3;
struct Color;

namespace Lights
{
   static constexpr int MaxShaderLights = 4; // Max dynamic lights supported by shader

    enum class LightTypes
    {
        Directional = 0,
        Point = 1,
        Spot = 2
    };

    class Light
    {
    public:
        Light(int id);
        virtual ~Light();

        virtual bool Update();

        bool IsDirty() const { return Dirty; }

        LightTypes GetType() const { return LightType; }

        void SetPosition(const Vector3& pos);
        void SetIntensity(const Color& color);
        void SetAttenuation(float attenuation);
        void SetFalloff(float falloff);

        virtual void OnBindToShader();

    protected:
        int ID = 0;

        LightTypes LightType = LightTypes::Directional;

        float Position[3] = {0,0,0};
        float Intensity[4] = {1,1,1,1};

        float Attenuation = 5;
        float Falloff = 10;

        // Shader locations
        int EnabledLoc = -1;
        int TypeLoc = -1;
        int PositionLoc = -1;
        int IntensityLoc = -1;
        int AttenuationLoc = -1;
        int FalloffLoc = -1;

        void SetDirty() { Dirty = true; }

        int GetShaderLocation(std::string_view field);

    private:
        bool Dirty = false;
        std::string FieldNameCache;
    };

    class PointLight : public Light
    {
    public:
        PointLight(int id) : Light(id) { LightType = LightTypes::Point; }
    };

    class DirectionalLight : public Light
    {
    public:
        DirectionalLight(int id);

        void SetDirection(const Vector3& dir);

        bool Update() override;
        void OnBindToShader() override;

    protected:
        float Direction[3] = {1,-1,1};
        int DirectionLoc = -1;
    };

    class SpotLight : public DirectionalLight
    {
    public:
        SpotLight(int id);

        void SetConeAngle(const float& cone);
        bool Update() override;
        void OnBindToShader() override;

    protected:
        float Cone = 45;
        int ConeLoc = -1;
    };

    void SetLightingShader(Shader shader);
    Shader GetLightingShader();

    void SetAmbientColor(Color color);

    Light* AddLight(LightTypes lightType);
    void RemoveLight(Light* light);

    void ClearLights(Light* light);

    void UpdateLights(const Camera3D& viewportCamear);
}