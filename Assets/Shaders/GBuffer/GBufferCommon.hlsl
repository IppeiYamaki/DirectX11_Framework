#ifndef ENGINE_GBUFFER_COMMON_HLSL
#define ENGINE_GBUFFER_COMMON_HLSL

// Include common definitions (cbuffers for World/View/Projection, etc.)
#include "../Common/Common.hlsl"

//==============================
// GBuffer出力構造体
//==============================
struct GBufferOutput
{
    float4 Albedo    : SV_Target0; // RGB: Albedo, A: Alpha
    float4 Normal    : SV_Target1; // RGB: World Normal (packed), A: Roughness
    float4 Position  : SV_Target2; // RGB: World Position, A: Metallic
    float4 Emission  : SV_Target3; // RGB: Emission, A: AO
};

//==============================
// Extended Vertex I/O (Tangent/Bitangent対応)
//==============================
struct VS_EXTENDED_IN
{
    float3 Position  : POSITION0;
    float3 Normal    : NORMAL0;
    float3 Tangent   : TANGENT0;
    float3 Bitangent : BINORMAL0;
    float2 TexCoord  : TEXCOORD0;
};

struct VS_EXTENDED_SKIN_IN
{
    float3 Position  : POSITION0;
    float3 Normal    : NORMAL0;
    float3 Tangent   : TANGENT0;
    float3 Bitangent : BINORMAL0;
    float2 TexCoord  : TEXCOORD0;
    
    uint4  BoneIndices : BLENDINDICES0;
    float4 BoneWeights : BLENDWEIGHT0;
};

struct PS_GBUFFER_IN
{
    float4 Position    : SV_POSITION;
    float3 PositionWS  : TEXCOORD0;
    float3 NormalWS    : TEXCOORD1;
    float3 TangentWS   : TEXCOORD2;
    float3 BitangentWS : TEXCOORD3;
    float2 TexCoord    : TEXCOORD4;
};

//==============================
// GBuffer-specific Constant Buffers
//==============================
struct PBRMaterialParams
{
    float4 Albedo;      // RGB: Albedo, A: Alpha
    float  Metallic;    // 金属度
    float  Roughness;   // 粗さ
    float  AO;          // アンビエントオクルージョン
    float  _pad0;
    
    float4 Emission;    // RGB: 発光色, A: 強度
    
    uint   Flags;       // マテリアルフラグ
    uint   _pad1;
    uint   _pad2;
    uint   _pad3;
};

cbuffer PBRMaterialBuffer : register(b3)
{
    PBRMaterialParams g_pbrMaterial;
}

// PBRマテリアルフラグ
static const uint PBR_FLAG_USE_ALBEDO_TEX    = 1u << 0;
static const uint PBR_FLAG_USE_NORMAL_TEX    = 1u << 1;
static const uint PBR_FLAG_USE_METALLIC_TEX  = 1u << 2;
static const uint PBR_FLAG_USE_ROUGHNESS_TEX = 1u << 3;
static const uint PBR_FLAG_USE_AO_TEX        = 1u << 4;
static const uint PBR_FLAG_USE_EMISSION_TEX  = 1u << 5;

//==============================
// Utility Functions
//==============================

// 位置変換
float4 TransformPositionWVP(float3 positionOS)
{
    float4 p = float4(positionOS, 1.0f);
    p = mul(p, g_world);
    p = mul(p, g_view);
    p = mul(p, g_projection);
    return p;
}

// ワールド位置変換
float3 TransformPositionWorld(float3 positionOS)
{
    float4 p = float4(positionOS, 1.0f);
    p = mul(p, g_world);
    return p.xyz;
}

// 法線をワールド空間に変換
float3 TransformNormalToWorld(float3 normalOS)
{
    float3 n = mul(float4(normalOS, 0.0f), g_world).xyz;
    return normalize(n);
}

// タンジェントをワールド空間に変換
float3 TransformTangentToWorld(float3 tangentOS)
{
    float3 t = mul(float4(tangentOS, 0.0f), g_world).xyz;
    return normalize(t);
}

// 法線マップからワールド法線を計算
float3 CalculateWorldNormalFromMap(float3 normalMap, float3 normalWS, float3 tangentWS, float3 bitangentWS)
{
    // 法線マップの値を[-1, 1]に変換
    float3 n = normalMap * 2.0f - 1.0f;
    
    // TBN行列を構築
    float3x3 TBN = float3x3(
        normalize(tangentWS),
        normalize(bitangentWS),
        normalize(normalWS)
    );
    
    // タンジェント空間からワールド空間へ変換
    return normalize(mul(n, TBN));
}

// 法線のパック（[-1,1] -> [0,1]）
float3 PackNormal(float3 normal)
{
    return normal * 0.5f + 0.5f;
}

// 法線のアンパック（[0,1] -> [-1,1]）
float3 UnpackNormal(float3 packed)
{
    return packed * 2.0f - 1.0f;
}

bool IsPBRFlagSet(uint flag)
{
    return (g_pbrMaterial.Flags & flag) != 0;
}

#endif // ENGINE_GBUFFER_COMMON_HLSL
