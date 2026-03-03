/// @file   TerrainBlendPS.hlsl
/// @brief  地形用マルチテクスチャブレンドピクセルシェーダー
/// @note   最大4レイヤーのテクスチャをブレンドウェイトで混合する
#include "Common/Common.hlsl"

//==============================
// Terrain Blend Constants
//==============================
static const uint MAX_TERRAIN_LAYERS = 4;
static const float WEIGHT_EPSILON = 0.0001f;  // ブレンドウェイト正規化用の許容誤差

cbuffer TerrainBlendBuffer : register(b7)
{
    // ブレンドウェイト（xyzw = Layer 0,1,2,3）
    float4 g_blendWeights;
    
    // UVスケール（タイリング倍率）
    float4 g_uvScales;
    
    // 各レイヤーの色調補正
    float4 g_layerTints[MAX_TERRAIN_LAYERS];
    
    // 有効レイヤー数とフラグ
    uint   g_activeLayerCount;
    uint   g_terrainFlags;
    uint   g_terrainPad0;
    uint   g_terrainPad1;
}

//==============================
// Terrain Textures (t0-t3)
//==============================
Texture2D g_TerrainLayer0 : register(t0);  // Layer 0（例：草）
Texture2D g_TerrainLayer1 : register(t1);  // Layer 1（例：土）
Texture2D g_TerrainLayer2 : register(t2);  // Layer 2（例：砂）
Texture2D g_TerrainLayer3 : register(t3);  // Layer 3（例：岩）

SamplerState g_TerrainSampler : register(s0);

float4 main(PS_IN In) : SV_Target
{
    float2 uv0 = In.TexCoord * g_uvScales.x;
    float2 uv1 = In.TexCoord * g_uvScales.y;
    float4 col0 = g_TerrainLayer0.Sample(g_TerrainSampler, uv0) * g_blendWeights.x;
    float4 col1 = g_TerrainLayer1.Sample(g_TerrainSampler, uv1) * g_blendWeights.y;
    float4 col = col0 + col1;

    
        // ライティング計算
    float3 normal = normalize(In.NormalWS);
    float3 materialDiffuse = col.rgb;
    
    // 拡張ライティングを計算
    float3 litColor = CalculateExtendedLighting(In.WorldPos, normal, materialDiffuse);
    
    // 拡張ライトが無い場合はレガシーライティングにフォールバック
    if (g_directionalLightCount == 0 && g_pointLightCount == 0 && g_spotLightCount == 0)
    {
        if (IsLightEnabled())
        {
            float NdotL = max(dot(normal, g_light.Direction), 0.0f);
            litColor = g_light.Diffuse.rgb * NdotL * materialDiffuse + g_light.Ambient.rgb * materialDiffuse;
        }
        else
        {
            litColor = materialDiffuse;
        }
    }

    return col = float4(litColor, col.a);

}

