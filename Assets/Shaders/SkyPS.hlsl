#include "Common/Common.hlsl"

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

float4 main(PS_IN In) : SV_TARGET
{
    float2 uv = In.TexCoord;
    uv.y = 1.0f - uv.y; // 上下反転

    float4 tex = g_texture.Sample(g_sampler, uv);

    // DefaultPS と同じ乗算（想定）
    float4 col = tex * In.Color;
    
            // ライティング計算
    float3 normal = normalize(In.NormalWS);
    float3 materialDiffuse = col.rgb;
    
    //// 拡張ライティングを計算
    //float3 litColor = CalculateExtendedLighting(In.WorldPos, normal, materialDiffuse);
    
    //// 拡張ライトが無い場合はレガシーライティングにフォールバック
    //if (g_directionalLightCount == 0 && g_pointLightCount == 0 && g_spotLightCount == 0)
    //{
    //    if (IsLightEnabled())
    //    {
    //        float NdotL = max(dot(normal, g_light.Direction), 0.0f);
    //        litColor = g_light.Diffuse.rgb * NdotL * materialDiffuse + g_light.Ambient.rgb * materialDiffuse;
    //    }
    //    else
    //    {
    //        litColor = materialDiffuse;
    //    }
    //}

    return col/* = float4(litColor, col.a)*/;
    

}
