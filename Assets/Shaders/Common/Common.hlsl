#ifndef ENGINE_COMMON_HLSL
#define ENGINE_COMMON_HLSL

// row-major mode for easier matrix usage
#pragma pack_matrix(row_major)

//==============================
// Flags
//==============================
static const uint MATERIAL_FLAG_USE_TEXTURE = 1u << 0;
static const uint MATERIAL_FLAG_ALPHA_TEST = 1u << 1;

static const uint LIGHT_FLAG_ENABLED = 1u << 0;

//==============================
// Light Constants
//==============================
static const uint MAX_DIRECTIONAL_LIGHTS = 4;
static const uint MAX_POINT_LIGHTS = 16;
static const uint MAX_SPOT_LIGHTS = 16;

//==============================
// Constant Buffers
//==============================
cbuffer WorldBuffer : register(b0)
{
    float4x4 g_world;
}

cbuffer ViewBuffer : register(b1)
{
    float4x4 g_view;
}

cbuffer ProjectionBuffer : register(b2)
{
    float4x4 g_projection;
}

struct MaterialParams
{
    float4  BaseColor;      // Diffuse color
    float4  Ambient;        // Ambient coefficient
    float4  Specular;       // Specular
    float4  Emissive;       // Emissive

    float   Shininess;      // Shininess
    float   AlphaCutoff;    // Default: 0.5
    uint    Flags;          // MATERIAL_FLAG_***
    uint    _pad0;          // 16B alignment
};

cbuffer MaterialBuffer : register(b3)
{
    MaterialParams g_material;
}

struct DirectionalLight
{
    float3  Direction; // World space light direction
    uint    Flags; // LIGHT_FLAG_***

    float4  Diffuse; // Light color
    float4  Ambient; // Ambient
};

cbuffer LightBuffer : register(b4)
{
    DirectionalLight g_light;
}

// Clip plane for planar reflection (b6)
cbuffer ClipPlaneBuffer : register(b6)
{
    float4 g_clipPlane;         // Clip plane (xyz = normal, w = distance)
    float  g_clipPlaneEnabled;  // Clip plane enabled flag (0.0 or 1.0)
    float  g_clipPad0;
    float  g_clipPad1;
    float  g_clipPad2;
}

//==============================
// Extended Light Structures
//==============================
struct PointLight
{
    float3  Position;       // World space position
    float   Range;          // Attenuation range

    float4  Diffuse;        // Light color (RGB) and intensity (A)

    float   ConstantAtten;  // Constant attenuation
    float   LinearAtten;    // Linear attenuation
    float   QuadraticAtten; // Quadratic attenuation
    uint    Flags;          // LIGHT_FLAG_***
};

struct SpotLight
{
    float3  Position;       // World space position
    float   Range;          // Attenuation range

    float3  Direction;      // Light direction (normalized)
    float   InnerCosAngle;  // cos(inner cone angle)

    float4  Diffuse;        // Light color (RGB) and intensity (A)

    float   OuterCosAngle;  // cos(outer cone angle)
    float   ConstantAtten;  // Constant attenuation
    float   LinearAtten;    // Linear attenuation
    float   QuadraticAtten; // Quadratic attenuation

    uint    Flags;          // LIGHT_FLAG_***
    uint    _pad0;
    uint    _pad1;
    uint    _pad2;
};

// Extended light buffer (b8)
// Note: b7 is reserved for material-specific buffers (e.g., WaterBuffer)
cbuffer ExtendedLightBuffer : register(b8)
{
    DirectionalLight g_directionalLights[MAX_DIRECTIONAL_LIGHTS];
    PointLight       g_pointLights[MAX_POINT_LIGHTS];
    SpotLight        g_spotLights[MAX_SPOT_LIGHTS];

    uint g_directionalLightCount;
    uint g_pointLightCount;
    uint g_spotLightCount;
    uint g_extLightPad0;
}

//==============================
// Vertex I/O
//==============================
struct VS_IN
{
    float3 Position : POSITION0;
    float3 Normal   : NORMAL0;
    float4 Color    : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

struct PS_IN
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR0;
    float2 TexCoord : TEXCOORD0;

    // For lighting
    float3 NormalWS : TEXCOORD1;
    
    // World position for clip plane (PS only)
    float3 WorldPos : TEXCOORD2;
};

//==============================
// Utility
//==============================

// Transform position to world space
float3 TransformToWorldSpace(float3 positionOS)
{
    return mul(float4(positionOS, 1.0f), g_world).xyz;
}

float4 TransformPosition(float3 positionOS)
{
    float4 p = float4(positionOS, 1.0f);
    p = mul(p, g_world);
    p = mul(p, g_view);
    p = mul(p, g_projection);
    return p;
}

float3 TransformNormalWS(float3 normalOS)
{
    // For uniform scale, just use world matrix
    float3 n = mul(float4(normalOS, 0.0f), g_world).xyz;
    return normalize(n);
}

bool IsMaterialFlagSet(uint flag)
{
    return (g_material.Flags & flag) != 0;
}

bool IsLightEnabled()
{
    return (g_light.Flags & LIGHT_FLAG_ENABLED) != 0;
}

//==============================
// Lighting Calculations
//==============================

// Calculate attenuation for point/spot lights
// Uses inverse-square falloff with constant, linear, and quadratic terms
float CalculateAttenuation(float distance, float range, float constant, float linearAtten, float quadratic)
{
    // Range check
    if (distance > range)
        return 0.0f;

    // Standard attenuation formula: 1 / (c + l*d + q*d^2)
    float attenuation = 1.0f / (constant + linearAtten * distance + quadratic * distance * distance);
    
    // Smooth falloff at range boundary
    float rangeFalloff = saturate(1.0f - (distance / range));
    rangeFalloff = rangeFalloff * rangeFalloff; // Quadratic falloff
    
    return attenuation * rangeFalloff;
}

// Calculate directional light contribution
float3 CalculateDirectionalLight(DirectionalLight light, float3 normal, float3 materialDiffuse)
{
    if ((light.Flags & LIGHT_FLAG_ENABLED) == 0)
        return float3(0.0f, 0.0f, 0.0f);

    // Diffuse (Lambert)
    float NdotL = max(dot(normal, -light.Direction), 0.0f);
    float3 diffuse = light.Diffuse.rgb * NdotL * materialDiffuse;

    // Ambient
    float3 ambient = light.Ambient.rgb * materialDiffuse;

    return diffuse + ambient;
}

// Calculate point light contribution
float3 CalculatePointLight(PointLight light, float3 worldPos, float3 normal, float3 materialDiffuse)
{
    if ((light.Flags & LIGHT_FLAG_ENABLED) == 0)
        return float3(0.0f, 0.0f, 0.0f);

    // Direction from surface to light
    float3 lightDir = light.Position - worldPos;
    float distance = length(lightDir);
    
    // Range check
    if (distance > light.Range)
        return float3(0.0f, 0.0f, 0.0f);

    lightDir = normalize(lightDir);

    // Attenuation
    float attenuation = CalculateAttenuation(distance, light.Range, 
        light.ConstantAtten, light.LinearAtten, light.QuadraticAtten);

    // Diffuse (Lambert)
    float NdotL = max(dot(normal, lightDir), 0.0f);
    float3 diffuse = light.Diffuse.rgb * NdotL * materialDiffuse * attenuation;

    return diffuse;
}

// Calculate spot light contribution
float3 CalculateSpotLight(SpotLight light, float3 worldPos, float3 normal, float3 materialDiffuse)
{
    if ((light.Flags & LIGHT_FLAG_ENABLED) == 0)
        return float3(0.0f, 0.0f, 0.0f);

    // Direction from surface to light
    float3 lightDir = light.Position - worldPos;
    float distance = length(lightDir);
    
    // Range check
    if (distance > light.Range)
        return float3(0.0f, 0.0f, 0.0f);

    lightDir = normalize(lightDir);

    // Spot cone attenuation
    // Calculate angle between light direction and direction to surface
    float spotCos = dot(-lightDir, light.Direction);
    
    // Check if outside outer cone
    if (spotCos < light.OuterCosAngle)
        return float3(0.0f, 0.0f, 0.0f);

    // Cone intensity falloff (smooth between inner and outer cone)
    float spotIntensity = saturate((spotCos - light.OuterCosAngle) / 
        max(light.InnerCosAngle - light.OuterCosAngle, 0.001f));
    // Smooth falloff
    spotIntensity = spotIntensity * spotIntensity;

    // Distance attenuation
    float attenuation = CalculateAttenuation(distance, light.Range,
        light.ConstantAtten, light.LinearAtten, light.QuadraticAtten);

    // Diffuse (Lambert)
    float NdotL = max(dot(normal, lightDir), 0.0f);
    float3 diffuse = light.Diffuse.rgb * NdotL * materialDiffuse * attenuation * spotIntensity;

    return diffuse;
}

// Calculate all extended lights contribution
float3 CalculateExtendedLighting(float3 worldPos, float3 normal, float3 materialDiffuse)
{
    float3 totalLight = float3(0.0f, 0.0f, 0.0f);

    // Directional lights
    for (uint i = 0; i < g_directionalLightCount && i < MAX_DIRECTIONAL_LIGHTS; ++i)
    {
        totalLight += CalculateDirectionalLight(g_directionalLights[i], normal, materialDiffuse);
    }

    // Point lights
    for (uint j = 0; j < g_pointLightCount && j < MAX_POINT_LIGHTS; ++j)
    {
        totalLight += CalculatePointLight(g_pointLights[j], worldPos, normal, materialDiffuse);
    }

    // Spot lights
    for (uint k = 0; k < g_spotLightCount && k < MAX_SPOT_LIGHTS; ++k)
    {
        totalLight += CalculateSpotLight(g_spotLights[k], worldPos, normal, materialDiffuse);
    }

    return totalLight;
}

// PS専用: アルファテストを適用（PixelShaderからのみ呼び出すこと）
void ApplyAlphaTestPS(inout float4 color)
{
    if (IsMaterialFlagSet(MATERIAL_FLAG_ALPHA_TEST))
    {
        clip(color.a - g_material.AlphaCutoff);
    }
}

// PS専用: クリップ平面を適用（PixelShaderからのみ呼び出すこと）
// Branchless implementation: when enabled=0, clip() always passes (value >= 0)
void ApplyClipPlanePS(float3 worldPos)
{
    // Clip plane equation: ax + by + cz + d = 0
    // If dot(pos, normal) + d < 0, discard
    float distance = dot(worldPos, g_clipPlane.xyz) + g_clipPlane.w;
    // When g_clipPlaneEnabled = 0.0, result = 1.0 (always passes)
    // When g_clipPlaneEnabled = 1.0, result = distance (normal clip behavior)
    clip(lerp(1.0f, distance, g_clipPlaneEnabled));
}

#endif // ENGINE_COMMON_HLSL
