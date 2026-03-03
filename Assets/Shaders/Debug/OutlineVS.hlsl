/// @file   OutlineVS.hlsl
/// @brief  アウトライン用頂点シェーダー（法線方向に頂点を拡大してアウトライン効果）

cbuffer OutlineConstants : register(b0) {
    float4x4 g_world;
    float4x4 g_view;
    float4x4 g_projection;
    float4   g_outlineColor;    // RGB + thickness
};

struct VSInput {
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

struct VSOutput {
    float4 Position : SV_POSITION;
    float3 Color    : COLOR0;
};

VSOutput main(VSInput input) {
    VSOutput output;
    
    float thickness = g_outlineColor.w;
    
    // 法線方向に頂点を拡大してアウトライン効果
    float3 expandedPos = input.Position + input.Normal * thickness;
    
    // ワールド変換
    float4 worldPos = mul(float4(expandedPos, 1.0f), g_world);
    
    // ビュー変換
    float4 viewPos = mul(worldPos, g_view);
    
    // プロジェクション変換
    output.Position = mul(viewPos, g_projection);
    
    // アウトライン色を出力
    output.Color = g_outlineColor.rgb;
    
    return output;
}
