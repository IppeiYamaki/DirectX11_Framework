#define NOMINMAX
#include "SampleCubeMaterial.h"

#include <cstddef>

#include "Engine/Core/Logger.h"
#include "Engine/Resources/AssetManager.h"

#include "Engine/Graphics/Material.h"
#include "Engine/Graphics/Shader.h"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"

namespace Game {

    // DefaultVSに合わせる入力レイアウト（Pos/Normal/Color/UV）
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

    std::shared_ptr<Engine::Material> SampleCubeMaterial::Create(const Engine::MaterialBuildContext& ctx) {
        if (!ctx.m_device || !ctx.m_assets) {
            Engine::Logger::Error("SampleCubeMaterial::Create failed: ctx invalid.");
            return {};
        }

        // いつものデフォルトシェーダ（Assets/Shaders/*.cso）
        auto vs = ctx.m_assets->LoadVertexShader(L"Shaders/DefaultVS.cso");
        auto ps = ctx.m_assets->LoadPixelShader(L"Shaders/DefaultPS.cso");
        if (!vs || !ps) return {};

        // InputLayout（名前でキャッシュされる想定）
        auto il = ctx.m_assets->CreateInputLayout(L"DefaultPosNormColorUv", CreateDefaultLayout(), *vs);
        if (!il) return {};

        auto mat = std::make_shared<Engine::Material>();
        if (!mat->Initialize(ctx.m_device)) return {};

        mat->SetVertexShader(vs);
        mat->SetPixelShader(ps);
        mat->SetInputLayout(il);

        // --- Material params ---
        mat->GetParams().m_baseColor = Engine::Vector4(1, 1, 1, 1);

        // --- Texture ---
        Engine::TextureLoadOptions opt{};
        opt.m_generateMipMaps = true;
        opt.m_forceSRgb = true; // 色テクスチャなら基本true（ガンマが正しくなる）

        // SetBaseDirectory(L"Assets") しているなら、ここは "Textures/..." でOK
        //auto tex = ctx.m_assets->LoadTexture(L"Textures/DefoTexture/Texture_White.png", opt);
        auto tex = ctx.m_assets->LoadTexture(L"Textures/Sample.png", opt);


        if (tex && tex->IsLoaded()) {
            mat->SetTexture(tex);

            // 念のため “使用フラグ” を明示で立てる（Material実装差異で事故りやすい所）
            mat->GetParams().m_flags |= Engine::kMaterialFlagUseTexture;

            // SetTexture がフラグを立てない実装でも確実にON
            mat->EnableTexture(true);

            Engine::Logger::Info("SampleCubeMaterial: texture applied.");
        }
        else {
            mat->EnableTexture(false);
            Engine::Logger::Warn("SampleCubeMaterial: texture load failed (fallback to color).");
        };

        return mat;
    }

} // namespace Game
