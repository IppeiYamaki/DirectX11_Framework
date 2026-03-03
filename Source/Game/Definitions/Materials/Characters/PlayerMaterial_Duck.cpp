#define NOMINMAX
#include "PlayerMaterial_Duck.h"

#include <cstddef>

#include "Engine/Core/Logger.h"
#include "Engine/Resources/AssetManager.h"

#include "Engine/Graphics/Material.h"
#include "Engine/Graphics/Shader.h"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"

namespace Game {

    // DuckPBRVSに合わせた頂点レイアウト（Pos/Normal/Color/UV）
    struct VertexPosNormColorUv {
        Engine::Vector3 m_pos;
        Engine::Vector3 m_normal;
        Engine::Vector4 m_color;
        Engine::Vector2 m_uv;
    };

    static Engine::VertexInputLayout CreateDefaultLayout() {
        Engine::VertexInputLayout layout;
        layout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, (UINT)offsetof(VertexPosNormColorUv, m_pos),    D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layout.push_back({ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, (UINT)offsetof(VertexPosNormColorUv, m_normal), D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layout.push_back({ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, (UINT)offsetof(VertexPosNormColorUv, m_color),  D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, (UINT)offsetof(VertexPosNormColorUv, m_uv),     D3D11_INPUT_PER_VERTEX_DATA, 0 });
        return layout;
    }

    std::shared_ptr<Engine::Material> PlayerMaterial_Duck::Create(const Engine::MaterialBuildContext& ctx) {
        if (!ctx.m_device || !ctx.m_assets) {
            Engine::Logger::Error("PlayerMaterial_Duck::Create failed: ctx invalid.");
            return {};
        }

        // Duck PBR用のシェーダーをロード
        auto vs = ctx.m_assets->LoadVertexShader(L"Shaders/DuckPBRVS.cso");
        auto ps = ctx.m_assets->LoadPixelShader(L"Shaders/DuckPBRPS.cso");
        if (!vs || !ps) {
            Engine::Logger::Error("PlayerMaterial_Duck::Create failed: Could not load DuckPBR shaders.");
            return {};
        }

        // InputLayout（名前でキャッシュ管理）
        auto il = ctx.m_assets->CreateInputLayout(L"DuckPBRPosNormColorUv", CreateDefaultLayout(), *vs);
        if (!il) return {};

        auto mat = std::make_shared<Engine::Material>();
        if (!mat->Initialize(ctx.m_device)) return {};

        mat->SetVertexShader(vs);
        mat->SetPixelShader(ps);
        mat->SetInputLayout(il);

        // --- Material params (PBRパラメータとして使用) ---
        // BaseColor = Albedo base color
        // Ambient.x = Metallic base value
        // Ambient.y = Roughness base value
        // Ambient.z = AO base value
        // Ambient.w = unused (padding)
        // Emissive = Emission
        mat->GetParams().m_baseColor = Engine::Vector4(1.0f, 1.0f, 1.0f, 1.0f);  // Albedo base (white, multiplied with texture)
        mat->GetParams().m_ambient = Engine::Vector4(0.0f, 0.5f, 1.0f, 1.0f);    // Metallic=0, Roughness=0.5, AO=1.0, w=unused
        
        // PBRフラグを設定（Albedo, Metallic, Roughness, Normal全て使用）
        // 注: シェーダー側のPBR_FLAG_* 定義と合わせる
        const std::uint32_t PBR_FLAG_USE_ALBEDO_TEX    = 1u << 0;
        const std::uint32_t PBR_FLAG_USE_NORMAL_TEX    = 1u << 1;
        const std::uint32_t PBR_FLAG_USE_METALLIC_TEX  = 1u << 2;
        const std::uint32_t PBR_FLAG_USE_ROUGHNESS_TEX = 1u << 3;
        
        mat->GetParams().m_flags = PBR_FLAG_USE_ALBEDO_TEX | PBR_FLAG_USE_NORMAL_TEX | 
                                   PBR_FLAG_USE_METALLIC_TEX | PBR_FLAG_USE_ROUGHNESS_TEX;

        // --- PBRテクスチャのロード ---
        
        // Albedo/Diffuseテクスチャ (sRGB)
        Engine::TextureLoadOptions albedoOpt{};
        albedoOpt.m_generateMipMaps = true;
        albedoOpt.m_forceSRgb = true;  // 色テクスチャはsRGB
        auto albedoTex = ctx.m_assets->LoadTexture(L"Textures/Characters/Duck/rubber_duck_toy_diff_4k.jpg", albedoOpt);
        if (albedoTex && albedoTex->IsLoaded()) {
            mat->SetTerrainLayerTexture(0, albedoTex);  // t0: Albedo
            Engine::Logger::Info("PlayerMaterial_Duck: Albedo texture loaded.");
        } else {
            Engine::Logger::Warn("PlayerMaterial_Duck: Albedo texture load failed.");
        }
        
        // Metallicテクスチャ (Linear)
        Engine::TextureLoadOptions metallicOpt{};
        metallicOpt.m_generateMipMaps = true;
        metallicOpt.m_forceSRgb = false;  // データテクスチャはLinear
        auto metallicTex = ctx.m_assets->LoadTexture(L"Textures/Characters/Duck/rubber_duck_toy_metal_4k.exr", metallicOpt);
        if (metallicTex && metallicTex->IsLoaded()) {
            mat->SetTerrainLayerTexture(1, metallicTex);  // t1: Metallic
            Engine::Logger::Info("PlayerMaterial_Duck: Metallic texture loaded.");
        } else {
            Engine::Logger::Warn("PlayerMaterial_Duck: Metallic texture load failed.");
        }
        
        // Roughnessテクスチャ (Linear)
        Engine::TextureLoadOptions roughnessOpt{};
        roughnessOpt.m_generateMipMaps = true;
        roughnessOpt.m_forceSRgb = false;  // データテクスチャはLinear
        auto roughnessTex = ctx.m_assets->LoadTexture(L"Textures/Characters/Duck/rubber_duck_toy_rough_4k.exr", roughnessOpt);
        if (roughnessTex && roughnessTex->IsLoaded()) {
            mat->SetTerrainLayerTexture(2, roughnessTex);  // t2: Roughness
            Engine::Logger::Info("PlayerMaterial_Duck: Roughness texture loaded.");
        } else {
            Engine::Logger::Warn("PlayerMaterial_Duck: Roughness texture load failed.");
        }
        
        // Normalテクスチャ (Linear)
        Engine::TextureLoadOptions normalOpt{};
        normalOpt.m_generateMipMaps = true;
        normalOpt.m_forceSRgb = false;  // 法線マップはLinear
        auto normalTex = ctx.m_assets->LoadTexture(L"Textures/Characters/Duck/rubber_duck_toy_nor_gl_4k.exr", normalOpt);
        if (normalTex && normalTex->IsLoaded()) {
            mat->SetTerrainLayerTexture(3, normalTex);  // t3: Normal
            Engine::Logger::Info("PlayerMaterial_Duck: Normal texture loaded.");
        } else {
            Engine::Logger::Warn("PlayerMaterial_Duck: Normal texture load failed.");
        }

        Engine::Logger::Info("PlayerMaterial_Duck: PBR material created successfully.");
        return mat;
    }

} // namespace Game
