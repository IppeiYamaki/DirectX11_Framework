#include "Common/Common.hlsl"

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

void main(in PS_IN In, out float4 outColor : SV_Target)
{
    // Apply clip plane for planar reflection (PS only)
    ApplyClipPlanePS(In.WorldPos);

    float4 color = In.Color;

    if (IsMaterialFlagSet(MATERIAL_FLAG_USE_TEXTURE))
    {
        float4 tex = g_Texture.Sample(g_SamplerState, In.TexCoord);
        color *= tex;
    }

    ApplyAlphaTestPS(color);

    // Apply extended lighting (Point, Spot, and Directional lights)
    float3 normal = normalize(In.NormalWS);
    float3 materialDiffuse = color.rgb;
    
    // Calculate lighting from all light sources
    float3 litColor = CalculateExtendedLighting(In.WorldPos, normal, materialDiffuse);
    
    // If no extended lights are active, fall back to legacy single directional light
    if (g_directionalLightCount == 0 && g_pointLightCount == 0 && g_spotLightCount == 0)
    {
        if (IsLightEnabled())
        {
            float NdotL = max(dot(normal, -g_light.Direction), 0.0f);
            litColor = g_light.Diffuse.rgb * NdotL * materialDiffuse + g_light.Ambient.rgb * materialDiffuse;
        }
        else
        {
            litColor = materialDiffuse;
        }
    }

    outColor = float4(litColor, color.a);
}
