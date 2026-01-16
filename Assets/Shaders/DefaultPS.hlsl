#include "Common/Common.hlsl"

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

void main(in PS_IN In, out float4 outColor : SV_Target)
{
    float4 color = In.Color;

    if (IsMaterialFlagSet(MATERIAL_FLAG_USE_TEXTURE))
    {
        float4 tex = g_Texture.Sample(g_SamplerState, In.TexCoord);
        color *= tex;
    }

    ApplyAlphaTest(color);

    outColor = color;
}
