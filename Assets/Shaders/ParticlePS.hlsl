#include "Common/Common.hlsl"

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

// Particle PS: Simple textured particle with color and alpha
float4 main(PS_IN In) : SV_Target
{
    // Apply clip plane (for reflection rendering)
    ApplyClipPlanePS(In.WorldPos);
    
    // Sample texture
    float4 texColor = float4(1, 1, 1, 1);
    if (IsMaterialFlagSet(MATERIAL_FLAG_USE_TEXTURE)) {
        texColor = g_Texture.Sample(g_SamplerState, In.TexCoord);
    }
    
    // Create circular particle shape using distance from center
    float2 centerDist = (In.TexCoord - 0.5f) * 2.0f;
    float dist = length(centerDist);
    
    // Soft circular falloff
    float alpha = 1.0f - smoothstep(0.0f, 1.0f, dist);
    alpha = pow(alpha, 2.0f); // Steeper falloff
    
    // Apply vertex color (brightness is in color intensity)
    float4 finalColor = In.Color * texColor;
    
    // Apply circular alpha
    finalColor.a *= alpha;
    
    // Increase brightness for glow effect
    finalColor.rgb *= 2.0f;
    
    return finalColor;
}
