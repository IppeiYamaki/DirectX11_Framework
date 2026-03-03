#include "Common/Common.hlsl"

// Particle VS: Billboard in world space
void main(in VS_IN In, out PS_IN Out)
{
    // Get particle world position from world matrix translation
    float3 particleWorldPos = float3(g_world._41, g_world._42, g_world._43);
    
    // Extract camera position from view matrix
    float3 cameraPos = float3(
        -g_view._41 * g_view._11 - g_view._42 * g_view._12 - g_view._43 * g_view._13,
        -g_view._41 * g_view._21 - g_view._42 * g_view._22 - g_view._43 * g_view._23,
        -g_view._41 * g_view._31 - g_view._42 * g_view._32 - g_view._43 * g_view._33
    );
    
    // Billboard: construct a quad facing the camera
    // In.TexCoord.xy stores the corner offset (0 to 1)
    float2 offset = (In.TexCoord.xy - 0.5f) * 2.0f; // Convert 0-1 to -1 to 1
    
    // Extract camera right and up vectors from view matrix
    float3 cameraRight = normalize(float3(g_view._11, g_view._21, g_view._31));
    float3 cameraUp = normalize(float3(g_view._12, g_view._22, g_view._32));
    
    // Calculate billboard vertex position
    // Scale is already in world matrix, so we just need unit offset
    float3 billboardPos = particleWorldPos + (cameraRight * offset.x) + (cameraUp * offset.y);
    
    // Transform to clip space
    float4 viewPos = mul(float4(billboardPos, 1.0f), g_view);
    Out.Position = mul(viewPos, g_projection);
    
    // Pass through texture coordinates
    Out.TexCoord = In.TexCoord;
    
    // Pass through color (with brightness)
    Out.Color = In.Color * g_material.BaseColor;
    
    // Set world position for clipping
    Out.WorldPos = billboardPos;
    
    // Normal (not really needed for particles, but required by PS_IN)
    Out.NormalWS = float3(0, 1, 0);
}
