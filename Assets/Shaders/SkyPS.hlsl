#include "Common/Common.hlsl"

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

float4 main(PS_IN In) : SV_TARGET
{
    float2 uv = In.TexCoord;
    uv.y = 1.0f - uv.y; // šã‰º”½“]

    float4 tex = g_texture.Sample(g_sampler, uv);

    // DefaultPS ‚Æ“¯‚¶æZi‘z’èj
    float4 col = tex * In.Color;
    return col;
}
