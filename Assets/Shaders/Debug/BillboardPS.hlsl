/// @file   BillboardPS.hlsl
/// @brief  ビルボード用ピクセルシェーダー（ライトアイコン描画）

cbuffer BillboardConstants : register(b0) {
    float4x4 g_view;        // カメラ軸抽出用のビュー行列
    float4x4 g_viewProj;    // 最終座標変換用のビュープロジェクション行列
    float4   g_position;    // xyz = world position, w = size
    float4   g_color;       // RGBA
    float4   g_direction;   // xyz = direction, w = show direction (1.0 = show arrow)
};

struct PSInput {
    float4 Position  : SV_POSITION;
    float2 TexCoord  : TEXCOORD0;
    float4 Color     : COLOR0;
    float4 Direction : TEXCOORD1;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

// 矢印を描画するヘルパー関数
// dir: ライトの3D方向ベクトルのXY成分（Direction.xyとして渡される）
// これはワールド空間の方向ベクトルであり、ビルボード上に2D投影される
float DrawArrow(float2 uv, float2 dir) {
    // UV座標を中心原点に変換
    float2 centeredUV = uv - 0.5f;
    
    // 方向ベクトルを2D化
    // dirはワールド空間の方向ベクトルのXY成分
    // Y軸を反転：ワールドY軸（上方向）とスクリーンY軸（下方向）の対応のため
    // これにより、上向きの方向は矢印が上を向くようになる
    float2 dir2D = normalize(float2(dir.x, -dir.y));
    
    // 矢印の方向に沿ってUV座標を回転
    float angle = atan2(dir2D.y, dir2D.x);
    float cosA = cos(angle);
    float sinA = sin(angle);
    
    float2 rotatedUV;
    rotatedUV.x = centeredUV.x * cosA + centeredUV.y * sinA;
    rotatedUV.y = -centeredUV.x * sinA + centeredUV.y * cosA;
    
    // 矢印形状を描画
    // 矢印の軸
    float lineWidth = 0.05f;
    float arrowLine = (abs(rotatedUV.y) < lineWidth && rotatedUV.x > -0.3f && rotatedUV.x < 0.2f) ? 1.0f : 0.0f;
    
    // 矢印の先端（三角形）
    float arrowHead = 0.0f;
    if (rotatedUV.x > 0.1f && rotatedUV.x < 0.4f) {
        float headWidth = (0.4f - rotatedUV.x) * 0.8f;
        arrowHead = (abs(rotatedUV.y) < headWidth) ? 1.0f : 0.0f;
    }
    
    return max(arrowLine, arrowHead);
}

float4 main(PSInput input) : SV_TARGET {
    float4 texColor = g_texture.Sample(g_sampler, input.TexCoord);
    
    // テクスチャ色とカラーを乗算
    float4 baseColor = texColor * input.Color;
    
    // 方向表示が有効な場合、矢印を合成
    if (input.Direction.w > 0.5f) {
        float arrow = DrawArrow(input.TexCoord, input.Direction.xy);
        // 矢印を白色で描画
        baseColor.rgb = lerp(baseColor.rgb, float3(1.0f, 1.0f, 0.0f), arrow * 0.7f);
    }
    
    // アルファテスト（テクスチャがない場合でも見えるように）
    if (baseColor.a < 0.1f) {
        // テクスチャがない場合はデフォルトの円形アイコンを描画
        float2 center = input.TexCoord - 0.5f;
        float dist = length(center);
        if (dist < 0.4f) {
            // 内側は明るく
            baseColor = float4(input.Color.rgb * 1.5f, 1.0f);
        } else if (dist < 0.5f) {
            // 縁取り
            baseColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
        } else {
            discard;
        }
        
        // 方向表示を追加
        if (input.Direction.w > 0.5f) {
            float arrow = DrawArrow(input.TexCoord, input.Direction.xy);
            baseColor.rgb = lerp(baseColor.rgb, float3(1.0f, 1.0f, 0.0f), arrow * 0.9f);
        }
    }
    
    return baseColor;
}
