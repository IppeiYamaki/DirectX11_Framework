/// @file   UnlitVS.hlsl
/// @brief  アンリット（無照明）頂点シェーダー（デバッグ用3Dオブジェクト表示）

cbuffer UnlitConstants : register(b0) {
    float4x4 g_world;       // ワールド行列
    float4x4 g_view;        // ビュー行列
    float4x4 g_projection;  // 射影行列
    float4   g_color;       // RGBA color
};

struct VSInput {
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float4 VertexColor : COLOR0;  // Vertex color from mesh (overridden by g_color in constant buffer)
    float2 TexCoord : TEXCOORD0;
};

struct VSOutput {
    float4 Position : SV_POSITION;
    float3 Normal   : NORMAL;
    float4 Color    : COLOR0;
};

VSOutput main(VSInput input) {
    VSOutput output;
    
    // Transform position to world space
    float4 worldPos = mul(float4(input.Position, 1.0f), g_world);
    
    // Transform to view space
    float4 viewPos = mul(worldPos, g_view);
    
    // Transform to clip space
    output.Position = mul(viewPos, g_projection);
    
    // Transform normal to world space for simple shading
    output.Normal = normalize(mul(input.Normal, (float3x3)g_world));
    
    // Pass color from constant buffer (ignore vertex color)
    output.Color = g_color;
    
    return output;
}
