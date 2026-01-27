#include "GBufferCommon.hlsl"

void main(in VS_EXTENDED_IN In, out PS_GBUFFER_IN Out)
{
    // ワールド座標変換
    Out.PositionWS = TransformPositionWorld(In.Position);
    
    // MVP変換
    Out.Position = TransformPositionWVP(In.Position);
    
    // 法線・タンジェント・バイタンジェントをワールド空間に変換
    Out.NormalWS = TransformNormalToWorld(In.Normal);
    Out.TangentWS = TransformTangentToWorld(In.Tangent);
    Out.BitangentWS = TransformTangentToWorld(In.Bitangent);
    
    // テクスチャ座標
    Out.TexCoord = In.TexCoord;
}
