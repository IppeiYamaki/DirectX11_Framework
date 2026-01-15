#ifndef ENGINE_COMMON_HLSL
#define ENGINE_COMMON_HLSL

// row-majorで統一（CPU側のXMFLOAT4X4をTransposeなしで送る運用にしやすい）
#pragma pack_matrix(row_major)

//==============================
// Flags
//==============================
static const uint MATERIAL_FLAG_USE_TEXTURE = 1u << 0;
static const uint MATERIAL_FLAG_ALPHA_TEST = 1u << 1;

static const uint LIGHT_FLAG_ENABLED = 1u << 0;

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
    float4  BaseColor;      // Diffuse相当（乗算の基色）
    float4  Ambient;        // 環境光係数
    float4  Specular;       // 予備（将来）
    float4  Emissive;       // 自己発光

    float   Shininess;      // 予備
    float   AlphaCutoff;    // 例: 0.5
    uint    Flags;          // MATERIAL_FLAG_***
    uint    _pad0;          // 16B境界合わせ
};

cbuffer MaterialBuffer : register(b3)
{
    MaterialParams g_material;
}

struct DirectionalLight
{
    float3  Direction; // ワールド空間。ライトが向いている方向（※光が進む向き）
    uint    Flags; // LIGHT_FLAG_***

    float4  Diffuse; // 光の色（強さ込み）
    float4  Ambient; // 環境光
};

cbuffer LightBuffer : register(b4)
{
    DirectionalLight g_light;
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

    // Lighting用（必要なシェーダだけ使う）
    float3 NormalWS : TEXCOORD1;
};

//==============================
// Utility
//==============================
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
    // 非一様スケールがある場合は inverse-transpose が理想。
    // 今は最小実装として world による回転のみ想定（w=0で平行移動を無視）。
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

void ApplyAlphaTest(inout float4 color)
{
    if (IsMaterialFlagSet(MATERIAL_FLAG_ALPHA_TEST))
    {
        clip(color.a - g_material.AlphaCutoff);
    }
}

#endif // ENGINE_COMMON_HLSL
