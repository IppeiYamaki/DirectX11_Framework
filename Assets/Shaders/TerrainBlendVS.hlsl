/// @file   TerrainBlendVS.hlsl
/// @brief  地形用マルチテクスチャブレンド頂点シェーダー
/// @note   DefaultVSと同じ構造だが、将来の拡張を考慮して分離
#include "Common/Common.hlsl"

void main(in VS_IN In, out PS_IN Out)
{
    // Calculate world position for clip plane (to be used in PS)
    float3 worldPos = TransformToWorldSpace(In.Position);
    Out.WorldPos = worldPos;

    Out.Position = TransformPosition(In.Position);
    Out.TexCoord = In.TexCoord;

    Out.Color = In.Color * g_material.BaseColor;
    Out.NormalWS = TransformNormalWS(In.Normal);
}
