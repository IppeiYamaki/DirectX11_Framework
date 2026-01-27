#include "GBufferCommon.hlsl"
#include "../Common/Common.hlsl"

// b5 : Bones
cbuffer BonesBuffer : register(b5)
{
    float4x4 g_bones[128];
}

void main(in VS_EXTENDED_SKIN_IN In, out PS_GBUFFER_IN Out)
{
    float4 p = float4(In.Position, 1.0f);
    float4 n = float4(In.Normal, 0.0f);
    float4 t = float4(In.Tangent, 0.0f);
    float4 b = float4(In.Bitangent, 0.0f);

    // スキニング処理
    float4 sp = 0;
    float4 sn = 0;
    float4 st = 0;
    float4 sb = 0;

    sp += mul(p, g_bones[In.BoneIndices.x]) * In.BoneWeights.x;
    sp += mul(p, g_bones[In.BoneIndices.y]) * In.BoneWeights.y;
    sp += mul(p, g_bones[In.BoneIndices.z]) * In.BoneWeights.z;
    sp += mul(p, g_bones[In.BoneIndices.w]) * In.BoneWeights.w;

    sn += mul(n, g_bones[In.BoneIndices.x]) * In.BoneWeights.x;
    sn += mul(n, g_bones[In.BoneIndices.y]) * In.BoneWeights.y;
    sn += mul(n, g_bones[In.BoneIndices.z]) * In.BoneWeights.z;
    sn += mul(n, g_bones[In.BoneIndices.w]) * In.BoneWeights.w;

    st += mul(t, g_bones[In.BoneIndices.x]) * In.BoneWeights.x;
    st += mul(t, g_bones[In.BoneIndices.y]) * In.BoneWeights.y;
    st += mul(t, g_bones[In.BoneIndices.z]) * In.BoneWeights.z;
    st += mul(t, g_bones[In.BoneIndices.w]) * In.BoneWeights.w;

    sb += mul(b, g_bones[In.BoneIndices.x]) * In.BoneWeights.x;
    sb += mul(b, g_bones[In.BoneIndices.y]) * In.BoneWeights.y;
    sb += mul(b, g_bones[In.BoneIndices.z]) * In.BoneWeights.z;
    sb += mul(b, g_bones[In.BoneIndices.w]) * In.BoneWeights.w;

    // ワールド座標変換
    Out.PositionWS = TransformPositionWorld(sp.xyz);
    
    // MVP変換
    Out.Position = TransformPositionWVP(sp.xyz);
    
    // 法線・タンジェント・バイタンジェントをワールド空間に変換
    Out.NormalWS = TransformNormalToWorld(sn.xyz);
    Out.TangentWS = TransformTangentToWorld(st.xyz);
    Out.BitangentWS = TransformTangentToWorld(sb.xyz);
    
    // テクスチャ座標
    Out.TexCoord = In.TexCoord;
}
