/// @file WaterVS.hlsl
/// @brief 水面用頂点シェーダー（XZ平面上の波の頂点変位と反射UV計算を含む）
/// @note  メッシュはXZ平面上に配置、Y軸が高さ方向、法線は(0,1,0)

#include "Common/Common.hlsl"

// 水面用定数バッファ (b7)
cbuffer WaterBuffer : register(b7)
{
    float g_time;           // 経過時間
    float g_waveSpeed;      // 波の速度
    float g_waveFrequency;  // 波の周波数
    float g_waveAmplitude;  // 波の振幅
    
    float g_uvScrollSpeed;  // UV スクロール速度
    float g_reflectionDistortion; // 反射の歪み強度
    float2 g_pad;
}

struct WaterVS_OUT
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR0;
    float2 TexCoord : TEXCOORD0;
    float3 NormalWS : TEXCOORD1;
    float4 ReflectionCoord : TEXCOORD2; // 反射テクスチャ座標
    float3 WorldPos : TEXCOORD3;
};

void main(in VS_IN In, out WaterVS_OUT Out)
{
    // XZ平面上の頂点位置
    float3 pos = In.Position;
    
    // 複数の波を合成（XZ平面上でY方向に変位）
    // Wave 1: X方向に伝播
    float wavePhase1 = pos.x * g_waveFrequency + g_time * g_waveSpeed;
    // Wave 2: Z方向に伝播（異なる周波数と速度）
    float wavePhase2 = pos.z * g_waveFrequency * 0.7f + g_time * g_waveSpeed * 1.3f;
    // Wave 3: 斜め方向に伝播（より複雑な波紋効果）
    float wavePhase3 = (pos.x + pos.z) * g_waveFrequency * 0.5f + g_time * g_waveSpeed * 0.8f;
    
    // 波の高さを計算（Y軸方向の変位）
    float waveHeight = sin(wavePhase1) * g_waveAmplitude + 
                       sin(wavePhase2) * g_waveAmplitude * 0.5f +
                       sin(wavePhase3) * g_waveAmplitude * 0.3f;
    pos.y += waveHeight;
    
    // 波による法線の変化を計算（XZ平面基準、法線は上向き(0,1,0)からの変位）
    float dx = cos(wavePhase1) * g_waveFrequency * g_waveAmplitude +
               cos(wavePhase3) * g_waveFrequency * 0.5f * g_waveAmplitude * 0.3f;
    float dz = cos(wavePhase2) * g_waveFrequency * 0.7f * g_waveAmplitude * 0.5f +
               cos(wavePhase3) * g_waveFrequency * 0.5f * g_waveAmplitude * 0.3f;
    float3 normal = normalize(float3(-dx, 1.0f, -dz));
    
    // ワールド座標
    float4 worldPos = mul(float4(pos, 1.0f), g_world);
    Out.WorldPos = worldPos.xyz;
    
    // ビュー・プロジェクション変換
    float4 viewPos = mul(worldPos, g_view);
    Out.Position = mul(viewPos, g_projection);
    
    // テクスチャ座標（スクロール）
    Out.TexCoord = In.TexCoord + float2(g_time * g_uvScrollSpeed * 0.1f, g_time * g_uvScrollSpeed * 0.05f);
    
    // 反射テクスチャ座標（スクリーン座標 -> UV変換）
    // NDC座標からUV座標へ：x' = (x/w + 1) / 2, y' = (1 - y/w) / 2
    Out.ReflectionCoord = Out.Position;
    
    // 色
    Out.Color = In.Color * g_material.BaseColor;
    
    // ワールド空間法線（XZ平面の水面なのでワールド変換後も上向き基準）
    Out.NormalWS = mul(float4(normal, 0.0f), g_world).xyz;
    Out.NormalWS = normalize(Out.NormalWS);
}
