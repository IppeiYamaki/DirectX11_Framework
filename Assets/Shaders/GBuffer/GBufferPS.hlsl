#include "GBufferCommon.hlsl"

// テクスチャとサンプラー
Texture2D g_AlbedoTexture    : register(t0);
Texture2D g_NormalTexture    : register(t1);
Texture2D g_MetallicTexture  : register(t2);
Texture2D g_RoughnessTexture : register(t3);
Texture2D g_AOTexture        : register(t4);
Texture2D g_EmissionTexture  : register(t5);

SamplerState g_LinearSampler : register(s0);

void main(in PS_GBUFFER_IN In, out GBufferOutput Out)
{
    // Albedo
    float4 albedo = g_pbrMaterial.Albedo;
    if (IsPBRFlagSet(PBR_FLAG_USE_ALBEDO_TEX))
    {
        albedo *= g_AlbedoTexture.Sample(g_LinearSampler, In.TexCoord);
    }
    
    // Alpha test
    clip(albedo.a - 0.01f);
    
    Out.Albedo = albedo;
    
    // Normal
    float3 worldNormal = normalize(In.NormalWS);
    if (IsPBRFlagSet(PBR_FLAG_USE_NORMAL_TEX))
    {
        float3 normalMap = g_NormalTexture.Sample(g_LinearSampler, In.TexCoord).xyz;
        worldNormal = CalculateWorldNormalFromMap(normalMap, In.NormalWS, In.TangentWS, In.BitangentWS);
    }
    
    // Roughness
    float roughness = g_pbrMaterial.Roughness;
    if (IsPBRFlagSet(PBR_FLAG_USE_ROUGHNESS_TEX))
    {
        roughness = g_RoughnessTexture.Sample(g_LinearSampler, In.TexCoord).r;
    }
    
    Out.Normal = float4(PackNormal(worldNormal), roughness);
    
    // Metallic
    float metallic = g_pbrMaterial.Metallic;
    if (IsPBRFlagSet(PBR_FLAG_USE_METALLIC_TEX))
    {
        metallic = g_MetallicTexture.Sample(g_LinearSampler, In.TexCoord).r;
    }
    
    Out.Position = float4(In.PositionWS, metallic);
    
    // AO
    float ao = g_pbrMaterial.AO;
    if (IsPBRFlagSet(PBR_FLAG_USE_AO_TEX))
    {
        ao = g_AOTexture.Sample(g_LinearSampler, In.TexCoord).r;
    }
    
    // Emission
    float4 emission = g_pbrMaterial.Emission;
    if (IsPBRFlagSet(PBR_FLAG_USE_EMISSION_TEX))
    {
        emission.rgb *= g_EmissionTexture.Sample(g_LinearSampler, In.TexCoord).rgb;
    }
    
    Out.Emission = float4(emission.rgb * emission.a, ao);
}
