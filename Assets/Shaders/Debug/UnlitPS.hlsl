/// @file   UnlitPS.hlsl
/// @brief  アンリット（無照明）ピクセルシェーダー（デバッグ用3Dオブジェクト表示）

struct PSInput {
    float4 Position : SV_POSITION;
    float3 Normal   : NORMAL;
    float4 Color    : COLOR0;
};

float4 main(PSInput input) : SV_TARGET {
    // Simple pseudo-shading based on normal direction
    // This gives the sphere a 3D look without full lighting calculations
    float3 lightDir = normalize(float3(0.5f, 1.0f, -0.5f));
    float ndotl = max(0.3f, dot(input.Normal, lightDir));
    
    float3 finalColor = input.Color.rgb * ndotl;
    
    // Add slight emission effect to make lights stand out
    finalColor += input.Color.rgb * 0.3f;
    
    return float4(saturate(finalColor), input.Color.a);
}
