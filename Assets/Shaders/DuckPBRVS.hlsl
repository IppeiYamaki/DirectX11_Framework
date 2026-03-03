#include "Common/Common.hlsl"

// 入力構造体 (Pos/Normal/Color/UV)
struct VS_INPUT
{
    float3 Position : POSITION0;
    float3 Normal   : NORMAL0;
    float4 Color    : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

// 出力構造体
struct PS_INPUT
{
    float4 Position   : SV_POSITION;
    float3 PositionWS : TEXCOORD0;
    float3 NormalWS   : TEXCOORD1;
    float2 TexCoord   : TEXCOORD2;
};

void main(in VS_INPUT In, out PS_INPUT Out)
{
    // ワールド座標変換
    float4 posWorld = mul(float4(In.Position, 1.0f), g_world);
    Out.PositionWS = posWorld.xyz;
    
    // MVP変換
    float4 posView = mul(posWorld, g_view);
    Out.Position = mul(posView, g_projection);
    
    // 法線をワールド空間に変換
    Out.NormalWS = normalize(mul(float4(In.Normal, 0.0f), g_world).xyz);
    
    // テクスチャ座標
    Out.TexCoord = In.TexCoord;
}
