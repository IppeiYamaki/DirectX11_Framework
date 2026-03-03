/// @file   OutlinePS.hlsl
/// @brief  アウトライン用ピクセルシェーダー（蛍光ネオン風アウトライン）

cbuffer OutlineConstants : register(b0) {
    float4x4 g_world;
    float4x4 g_view;
    float4x4 g_projection;
    float4   g_outlineColor;    // RGB + thickness
};

struct PSInput {
    float4 Position : SV_POSITION;
    float3 Color    : COLOR0;
};

float4 main(PSInput input) : SV_TARGET {
    // 蛍光ネオン風の効果（高輝度）
    float3 neonColor = input.Color * 1.5f;  // 輝度を上げる
    
    // グロー効果のため、若干のアルファブレンド
    return float4(neonColor, 1.0f);
}
