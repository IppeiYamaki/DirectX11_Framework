/// @file   UIVS.hlsl
/// @brief  UI用頂点シェーダー（2Dスクリーン座標での描画）

struct VSInput {
    float2 Position : POSITION;   // スクリーン座標 (ピクセル)
    float2 TexCoord : TEXCOORD0;
};

struct VSOutput {
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

VSOutput main(VSInput input) {
    VSOutput output;
    
    // スクリーン座標をNDC (-1~1) に変換
    // x: 0 -> -1, screenWidth -> 1
    // y: 0 -> 1, screenHeight -> -1 (Y軸反転)
    float2 ndc;
    ndc.x = (input.Position.x * invScreenWidth) * 2.0f - 1.0f;
    ndc.y = 1.0f - (input.Position.y * invScreenHeight) * 2.0f;
    
    output.Position = float4(ndc, 0.0f, 1.0f);
    output.TexCoord = input.TexCoord;
    
    return output;
}
