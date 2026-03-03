/// @file   UIPS.hlsl
/// @brief  UI用ピクセルシェーダー（テクスチャ描画 + 乗算カラー）

struct PSInput {
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

cbuffer UIConstants : register(b0) {
    float screenWidth;
    float screenHeight;
    float invScreenWidth;
    float invScreenHeight;
    float4 g_color;       // RGBA乗算カラー
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

float4 main(PSInput input) : SV_TARGET {
    float4 texColor = g_texture.Sample(g_sampler, input.TexCoord);
    return texColor * g_color;
}
