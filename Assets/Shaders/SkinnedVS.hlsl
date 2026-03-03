#include "Common/Common.hlsl"

// b5 : Bones
cbuffer BonesBuffer : register(b5)
{
    float4x4 g_bones[128];
}

struct VS_SKIN_IN
{
    float3 Position : POSITION0;
    float3 Normal : NORMAL0;
    float4 Color : COLOR0;
    float2 TexCoord : TEXCOORD0;

    uint4 BoneIndices : BLENDINDICES0;
    float4 BoneWeights : BLENDWEIGHT0;
};

void main(in VS_SKIN_IN In, out PS_IN Out)
{
    float4 p = float4(In.Position, 1.0f);
    float4 n = float4(In.Normal, 0.0f);

    // skinning
    float4 sp = 0;
    float4 sn = 0;

    sp += mul(p, g_bones[In.BoneIndices.x]) * In.BoneWeights.x;
    sp += mul(p, g_bones[In.BoneIndices.y]) * In.BoneWeights.y;
    sp += mul(p, g_bones[In.BoneIndices.z]) * In.BoneWeights.z;
    sp += mul(p, g_bones[In.BoneIndices.w]) * In.BoneWeights.w;

    sn += mul(n, g_bones[In.BoneIndices.x]) * In.BoneWeights.x;
    sn += mul(n, g_bones[In.BoneIndices.y]) * In.BoneWeights.y;
    sn += mul(n, g_bones[In.BoneIndices.z]) * In.BoneWeights.z;
    sn += mul(n, g_bones[In.BoneIndices.w]) * In.BoneWeights.w;

    // Calculate world position for clip plane (to be used in PS)
    Out.WorldPos = TransformToWorldSpace(sp.xyz);

    // output
    Out.Position = TransformPosition(sp.xyz);
    Out.TexCoord = In.TexCoord;

    Out.Color = In.Color * g_material.BaseColor;
    Out.NormalWS = TransformNormalWS(sn.xyz);
}
