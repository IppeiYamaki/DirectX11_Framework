#include "Common/Common.hlsl"

// PBRマテリアルフラグ（MaterialParams.Flagsで使用）
static const uint PBR_FLAG_USE_ALBEDO_TEX    = 1u << 0;  // Same as MATERIAL_FLAG_USE_TEXTURE
static const uint PBR_FLAG_USE_NORMAL_TEX    = 1u << 1;  // Same as MATERIAL_FLAG_ALPHA_TEST (repurposed)
static const uint PBR_FLAG_USE_METALLIC_TEX  = 1u << 2;
static const uint PBR_FLAG_USE_ROUGHNESS_TEX = 1u << 3;

// テクスチャとサンプラー
Texture2D g_AlbedoTexture    : register(t0);
Texture2D g_MetallicTexture  : register(t1);
Texture2D g_RoughnessTexture : register(t2);
Texture2D g_NormalTexture    : register(t3);

SamplerState g_LinearSampler : register(s0);

// 入力構造体
struct PS_INPUT
{
    float4 Position   : SV_POSITION;
    float3 PositionWS : TEXCOORD0;
    float3 NormalWS   : TEXCOORD1;
    float2 TexCoord   : TEXCOORD2;
};

// 法線マップから法線を計算（微分を使ったTBN計算）
float3 CalculateNormalFromMap(float3 normalMap, float3 normalWS, float3 posWS, float2 uv)
{
    // 法線マップの値を[-1, 1]に変換
    float3 n = normalMap * 2.0f - 1.0f;
    
    // 位置とUVの微分からTBNを計算
    float3 dPdx = ddx(posWS);
    float3 dPdy = ddy(posWS);
    float2 dUVdx = ddx(uv);
    float2 dUVdy = ddy(uv);
    
    // タンジェントとビタンジェントを計算
    float3 N = normalize(normalWS);
    float3 T = normalize(dPdx * dUVdy.y - dPdy * dUVdx.y);
    float3 B = -normalize(cross(N, T));
    
    // TBN行列を構築
    float3x3 TBN = float3x3(T, B, N);
    
    // タンジェント空間からワールド空間へ変換
    return normalize(mul(n, TBN));
}

// シンプルなPBRライティング計算
float3 CalculatePBRLighting(
    float3 albedo,
    float metallic,
    float roughness,
    float3 normal,
    float3 viewDir,
    float3 lightDir,
    float3 lightColor
)
{
    float3 halfway = normalize(viewDir + lightDir);
    
    // Lambert diffuse
    float NdotL = max(dot(normal, lightDir), 0.0f);
    
    // Specular (Blinn-Phong approximation)
    float NdotH = max(dot(normal, halfway), 0.0f);
    float roughness2 = roughness * roughness;
    float specPower = max(2.0f / (roughness2 * roughness2) - 2.0f, 0.1f);
    float spec = pow(NdotH, specPower);
    
    // F0 (基底反射率): 金属は albedo、非金属は 0.04
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metallic);
    
    // Fresnel (Schlick近似)
    float VdotH = max(dot(viewDir, halfway), 0.0f);
    float3 F = F0 + (1.0f - F0) * pow(1.0f - VdotH, 5.0f);
    
    // Diffuse (金属度が高いほど拡散反射が減る)
    float3 kD = (1.0f - F) * (1.0f - metallic);
    float3 diffuse = kD * albedo / 3.14159f;
    
    // Specular
    float3 specular = F * spec * 0.25f;
    
    // Final lighting
    return (diffuse + specular) * lightColor * NdotL;
}

float4 main(in PS_INPUT In) : SV_TARGET
{
    // MaterialParams mapping to PBR:
    // g_material.BaseColor = Albedo
    // g_material.Ambient.x = Metallic
    // g_material.Ambient.y = Roughness
    // g_material.Ambient.z = AO
    // g_material.Emissive = Emission
    // g_material.Flags = PBR flags
    
    
    // Albedo (sRGB -> Linear変換はテクスチャロード時に行われる想定)
    float4 albedo = g_material.BaseColor;
    float2 uv = float2(1.0 - In.TexCoord.x, 1.0 - In.TexCoord.y);

    if ((g_material.Flags & PBR_FLAG_USE_ALBEDO_TEX) != 0)
    {
        albedo *= g_AlbedoTexture.Sample(g_LinearSampler, uv);
    }
    
    // Alpha test
    clip(albedo.a - 0.01f);
    
    // Metallic
    float metallic = g_material.Ambient.x;
    if ((g_material.Flags & PBR_FLAG_USE_METALLIC_TEX) != 0)
    {
        metallic = g_MetallicTexture.Sample(g_LinearSampler, uv).r;
    }
    
    // Roughness
    float roughness = g_material.Ambient.y;
    if ((g_material.Flags & PBR_FLAG_USE_ROUGHNESS_TEX) != 0)
    {
        roughness = g_RoughnessTexture.Sample(g_LinearSampler, uv).r;
    }
    
    // AO (Ambient Occlusion)
    // Note: Currently only uses constant AO value from material params.
    // Texture-based AO is not implemented in this version.
    float ao = g_material.Ambient.z;
    
    // Normal
    float3 worldNormal = normalize(In.NormalWS);
    if ((g_material.Flags & PBR_FLAG_USE_NORMAL_TEX) != 0)
    {
        float3 normalMap = g_NormalTexture.Sample(g_LinearSampler, uv).xyz;
        worldNormal = CalculateNormalFromMap(normalMap, In.NormalWS, In.PositionWS, uv);
    }
    
    // View direction (extract camera position from view matrix)
    // Camera position is the inverse translation of the view matrix
    float3 cameraPos = float3(
        -dot(g_view._m00_m01_m02, g_view._m30_m31_m32),
        -dot(g_view._m10_m11_m12, g_view._m30_m31_m32),
        -dot(g_view._m20_m21_m22, g_view._m30_m31_m32)
    );
    float3 viewDir = normalize(cameraPos - In.PositionWS);
    
    // ライティング計算
    // メインライト（太陽光的な方向光）
    float3 mainLightDir = normalize(float3(0.5f, 1.0f, 0.3f));
    float3 mainLightColor = float3(1.0f, 0.98f, 0.95f) * 1.5f;
    
    float3 lighting = CalculatePBRLighting(
        albedo.rgb,
        metallic,
        roughness,
        worldNormal,
        viewDir,
        mainLightDir,
        mainLightColor
    );
    
    // Ambient lighting (シンプルな環境光)
    float3 ambient = albedo.rgb * float3(0.3f, 0.35f, 0.4f) * ao;
    
    // Final color
    float3 finalColor = lighting + ambient;
    
    // Tone mapping (シンプルなReinhard)
    finalColor = finalColor / (finalColor + 1.0f);
    
    // Gamma correction (Linear -> sRGB)
    // NOTE: レンダーターゲットがsRGBフォーマットの場合は自動変換される
    // finalColor = pow(finalColor, 1.0f / 2.2f);
    
    return float4(finalColor, albedo.a);
}
