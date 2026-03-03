/// @file WaterPS.hlsl
/// @brief 水面用ピクセルシェーダー（XZ平面上の水面の反射テクスチャサンプリングと歪み）
/// @note  メッシュはXZ平面上に配置、Y軸が高さ方向、法線は(0,1,0)基準
/// @note  反射テクスチャとしてSky.pngを使用（将来的にPlanar Reflection RTに切り替え可能）

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

// 反射テクスチャ (t1)
Texture2D g_ReflectionTexture : register(t1);
SamplerState g_ReflectionSampler : register(s1);

// ベーステクスチャ（法線マップなど用）
Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

struct WaterPS_IN
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR0;
    float2 TexCoord : TEXCOORD0;
    float3 NormalWS : TEXCOORD1;
    float4 ReflectionCoord : TEXCOORD2;
    float3 WorldPos : TEXCOORD3;
};

void main(in WaterPS_IN In, out float4 outColor : SV_Target)
{
    // 反射UV計算
    // Sky.pngを反射として使用する場合、画面座標をそのまま使用するのではなく、
    // 法線と視線から反射方向を計算してテクスチャをサンプリングする
    
    // 基本の反射UV（スクリーン座標ベース）
    float2 reflectionUV;
    reflectionUV.x = (In.ReflectionCoord.x / In.ReflectionCoord.w) * 0.5f + 0.5f;
    reflectionUV.y = (-In.ReflectionCoord.y / In.ReflectionCoord.w) * 0.5f + 0.5f; // Y反転
    
    // 法線による歪み（XZ平面の水面なのでxz成分を使用）
    float2 distortion = In.NormalWS.xz * g_reflectionDistortion;
    reflectionUV += distortion;
    
    // 波による追加の歪み（XZ座標基準）
    float ripple = sin(In.WorldPos.x * 2.0f + g_time * 3.0f) * 0.005f +
                   cos(In.WorldPos.z * 2.5f + g_time * 2.5f) * 0.005f;
    reflectionUV += float2(ripple, ripple);
    
    // UV範囲をクランプではなくラップ（Sky.pngは環境マップとして使用）
    reflectionUV = frac(reflectionUV);
    
    // 反射テクスチャをサンプリング
    float4 reflectionColor = g_ReflectionTexture.Sample(g_ReflectionSampler, reflectionUV);
    
    // ベースカラー
    float4 baseColor = In.Color;
    
    // フレネル効果（XZ平面の水面：上から見ると透明、浅い角度で見ると反射が強い）
    // 水面法線は主に上向き(0,1,0)なので、Y成分が重要
    float3 viewDir = normalize(float3(0, 1, 0)); // 簡易的にY軸方向（上向き）
    float fresnel = pow(1.0f - saturate(dot(In.NormalWS, viewDir)), 2.0f);
    fresnel = saturate(fresnel * 0.8f + 0.2f); // 最低限の反射を確保
    
    // 水の色（深い青緑）
    float4 waterColor = float4(0.1f, 0.3f, 0.4f, 0.6f);
    
    // 反射と水色をブレンド
    float4 finalColor;
    finalColor.rgb = lerp(waterColor.rgb, reflectionColor.rgb, fresnel * 0.7f);
    finalColor.a = waterColor.a + fresnel * 0.3f;
    
    // マテリアルカラーを適用
    finalColor *= baseColor;
    
    outColor = finalColor;
}
