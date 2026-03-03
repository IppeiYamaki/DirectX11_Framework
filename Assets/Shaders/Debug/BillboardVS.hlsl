/// @file   BillboardVS.hlsl
/// @brief  ビルボード用頂点シェーダー（常にカメラ向きのスプライト表示）

cbuffer BillboardConstants : register(b0) {
    float4x4 g_view;        // カメラ軸抽出用のビュー行列
    float4x4 g_viewProj;    // 最終座標変換用のビュープロジェクション行列
    float4   g_position;    // xyz = world position, w = size
    float4   g_color;       // RGBA
    float4   g_direction;   // xyz = direction, w = show direction (1.0 = show arrow)
};

struct VSInput {
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
};

struct VSOutput {
    float4 Position  : SV_POSITION;
    float2 TexCoord  : TEXCOORD0;
    float4 Color     : COLOR0;
    float4 Direction : TEXCOORD1;
};

VSOutput main(VSInput input) {
    VSOutput output;
    
    float size = g_position.w;
    
    // ビルボード用：ビュー行列からカメラの右方向と上方向を抽出
    // ビュー行列の各行はカメラ座標系の基底ベクトルを表す（転置形式）
    // row 0: カメラの右方向（X軸）
    // row 1: カメラの上方向（Y軸）
    // row 2: カメラの前方向（Z軸、視線方向と逆）
    float3 right = normalize(float3(g_view._11, g_view._21, g_view._31));
    float3 up    = normalize(float3(g_view._12, g_view._22, g_view._32));
    
    // ビルボード頂点をワールド空間で計算
    float3 worldPos = g_position.xyz 
                    + right * input.Position.x * size 
                    + up * input.Position.y * size;
    
    // 最終位置を計算
    output.Position = mul(float4(worldPos, 1.0f), g_viewProj);
    output.TexCoord = input.TexCoord;
    output.Color = g_color;
    output.Direction = g_direction;
    
    return output;
}
