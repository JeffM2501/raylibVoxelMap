#include "lighting_system.h"
#include "raylib.h"

#include <array>

namespace Lights
{
    Shader LightShader = { 0 };

    std::array<Light*, MaxShaderLights> LightList;

    static constexpr char ViewPosName[] = "viewPos";
    static constexpr char EnabledName[] = "enabled";
    static constexpr char TypedName[] = "type";
    static constexpr char PositionName[] = "position";
    static constexpr char DirectionName[] = "direction";
    static constexpr char ColorName[] = "color";
    static constexpr char AttenuationName[] = "attenuation";
    static constexpr char FallofName[] = "falloff";
    static constexpr char ConeName[] = "cone";
    static constexpr char AmbientName[] = "ambient";

    static float ColorScale = 1.0f / 255.0f;

    static float Ambient[4] = { 0.05f ,0.05f, 0.05f, 1.0f };
    static int AmbientLoc = -1;

    void SetLightingShader(Shader shader)
    {
        LightShader = shader;
        LightShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(LightShader, ViewPosName);

        AmbientLoc = GetShaderLocation(shader, AmbientName);

        for (int i = 0; i < MaxShaderLights; i++)
        {
            if (LightList[i] == nullptr)
                continue;

            LightList[i]->OnBindToShader();
        }
    }

    Shader GetLightingShader()
    {
        return LightShader;
    }

    Light* AddLight(LightTypes lightType)
    {
        Light* newLight = nullptr;

        int id = -1;
        for (int i = 0; i < MaxShaderLights; i++)
        {
            if (LightList[i] == nullptr)
            {
                id = i;
                break;
            }
        }
        if (id == -1)
            return newLight;

        switch (lightType)
        {
        default:
            return nullptr;

        case LightTypes::Point:
            newLight = new PointLight(id);
            break;

        case LightTypes::Directional:
            newLight = new DirectionalLight(id);
            break;

        case LightTypes::Spot:
            newLight = new SpotLight(id);
            break;
        }
        newLight->OnBindToShader();
        LightList[id] = newLight;
        return newLight;
    }

    void RemoveLight(Light* light)
    {
        for (int i = 0; i < MaxShaderLights; i++)
        {
            if (LightList[i] == light)
            {
                delete(light);
                LightList[i] = nullptr;
                return;
            }
        }
    }

    void ClearLights(Light* light)
    {
        for (int i = 0; i < MaxShaderLights; i++)
        {
            delete(LightList[i]);
            LightList[i] = nullptr;
        }
    }

    void SetAmbientColor(Color color)
    {
        Ambient[0] = color.r * ColorScale;
        Ambient[1] = color.g * ColorScale;
        Ambient[2] = color.b * ColorScale;
    }

    void UpdateLights(const Camera3D& viewportCamera)
    {
        if (!IsShaderValid(LightShader))
            return;
        SetShaderValue(LightShader, AmbientLoc, Ambient, SHADER_UNIFORM_VEC4);

        SetShaderValue(LightShader, LightShader.locs[SHADER_LOC_VECTOR_VIEW], &viewportCamera.position, SHADER_UNIFORM_VEC3);

        for (int i = 0; i < MaxShaderLights; i++)
        {
            if (LightList[i] == nullptr)
                continue;

            if (LightList[i]->IsDirty())
                LightList[i]->Update();
        }
    }

    Light::Light(int id) : ID(id)
    {
    }

    Light::~Light()
    {
        // disable on destroy
        if (IsShaderValid(LightShader) && EnabledLoc > 0)
        {
            int enabled = 0;
            SetShaderValue(LightShader, EnabledLoc, &enabled, SHADER_UNIFORM_INT);
        }
    }

    bool Light::Update()
    {
        if (!IsShaderValid(LightShader) || !IsDirty())
            return false;

        Dirty = false;

        int enabled = 1;
        int type = int(LightType);

        SetShaderValue(LightShader, EnabledLoc, &enabled, SHADER_UNIFORM_INT);
        SetShaderValue(LightShader, TypeLoc, &type, SHADER_UNIFORM_INT);

        SetShaderValue(LightShader, PositionLoc, Position, SHADER_UNIFORM_VEC3);

        SetShaderValue(LightShader, IntensityLoc, Intensity, SHADER_UNIFORM_VEC4);

        SetShaderValue(LightShader, AttenuationLoc, &Attenuation, SHADER_UNIFORM_FLOAT);

        SetShaderValue(LightShader, FalloffLoc, &Falloff, SHADER_UNIFORM_FLOAT);

        return true;
    }

    void Light::SetPosition(const Vector3& pos)
    {
        Position[0] = pos.x;
        Position[1] = pos.y;
        Position[2] = pos.z;

        SetDirty();
    }

    void Light::SetIntensity(const Color& color)
    {
        Intensity[0] = color.r * ColorScale;
        Intensity[1] = color.g * ColorScale;
        Intensity[2] = color.b * ColorScale;

        SetDirty();
    }

    void Light::SetAttenuation(float attenuation)
    {
        Attenuation = attenuation;
        SetDirty();
    }

    void Light::SetFalloff(float falloff)
    {
        Falloff = falloff;
        SetDirty();
    }

    int Light::GetShaderLocation(std::string_view field)
    {
        FieldNameCache = "lights[" + std::to_string(ID) + "]." + field.data();

        return ::GetShaderLocation(LightShader, FieldNameCache.c_str());
    }

    void Light::OnBindToShader()
    {
        SetDirty();
        if (!IsShaderValid(LightShader))
            return;

        EnabledLoc = GetShaderLocation(EnabledName);
        TypeLoc = GetShaderLocation(TypedName);
        PositionLoc = GetShaderLocation(PositionName);
        IntensityLoc = GetShaderLocation(ColorName);
        AttenuationLoc = GetShaderLocation(AttenuationName);
        FalloffLoc = GetShaderLocation(FallofName);
    }

    ///--------DirectionalLight-------------

    DirectionalLight::DirectionalLight(int id)
        :Light(id)
    {
        LightType = LightTypes::Directional;
    }

    void DirectionalLight::SetDirection(const Vector3& dir)
    {
        Direction[0] = dir.x;
        Direction[1] = dir.y;
        Direction[2] = dir.z;
        SetDirty();
    }

    bool DirectionalLight::Update()
    {
        if (!Light::Update())
            return false;

        SetShaderValue(LightShader, DirectionLoc, Direction, SHADER_UNIFORM_VEC3);

        return true;
    }

    void DirectionalLight::OnBindToShader()
    {
        Light::OnBindToShader();
        DirectionLoc = GetShaderLocation(DirectionName);
    }

    ///--------SpotLight-------------

    SpotLight::SpotLight(int id) 
        : DirectionalLight(id)
    {
        LightType = LightTypes::Spot;
    }

    void SpotLight::SetConeAngle(const float &cone)
    {
        Cone = cone;
        SetDirty();
    }

    bool SpotLight::Update()
    {
        if (!DirectionalLight::Update())
            return false;

        SetShaderValue(LightShader, ConeLoc, &Cone, SHADER_UNIFORM_FLOAT);

        return true;
    }

    void SpotLight::OnBindToShader()
    {
        DirectionalLight::OnBindToShader();
        ConeLoc = GetShaderLocation(ConeName);
    }
}
