#define NOMINMAX
#include "TitleQuadMaterial.h"

#include <cstddef>

#include "Engine/Core/Logger.h"
#include "Engine/Resources/AssetManager.h"

#include "Engine/Graphics/Material.h"
#include "Engine/Graphics/Shader.h"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"

#include "Engine/Materials/MaterialBuildContext.h"

namespace Game {

    struct VertexPosNormColorUv {
        Engine::Vector3 m_pos;
        Engine::Vector3 m_normal;
        Engine::Vector4 m_color;
        Engine::Vector2 m_uv;
    };

    static Engine::VertexInputLayout CreateTitleQuadLayout() {
        Engine::VertexInputLayout layout;
        layout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, (UINT)offsetof(VertexPosNormColorUv, m_pos),    D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layout.push_back({ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, (UINT)offsetof(VertexPosNormColorUv, m_normal), D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layout.push_back({ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, (UINT)offsetof(VertexPosNormColorUv, m_color),  D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, (UINT)offsetof(VertexPosNormColorUv, m_uv),     D3D11_INPUT_PER_VERTEX_DATA, 0 });
        return layout;
    }

    std::shared_ptr<Engine::Material> TitleQuadMaterial::Create(const Engine::MaterialBuildContext& ctx) {
        if (!ctx.m_device || !ctx.m_assets) {
            Engine::Logger::Error("TitleQuadMaterial::Create failed: ctx invalid.");
            return {};
        }

        // DefaultVS/DefaultPS を使用（Unlit表示：ライト計算なし、テクスチャ色そのまま）
        auto vs = ctx.m_assets->LoadVertexShader(L"Shaders/DefaultVS.cso");
        auto ps = ctx.m_assets->LoadPixelShader(L"Shaders/DefaultPS.cso");
        if (!vs || !ps) {
            Engine::Logger::Error("TitleQuadMaterial::Create failed: shader load failed.");
            return {};
        }

        auto il = ctx.m_assets->CreateInputLayout(L"TitleQuadLayout", CreateTitleQuadLayout(), *vs);
        if (!il) {
            Engine::Logger::Error("TitleQuadMaterial::Create failed: input layout creation failed.");
            return {};
        }

        auto mat = std::make_shared<Engine::Material>();
        if (!mat->Initialize(ctx.m_device)) {
            Engine::Logger::Error("TitleQuadMaterial::Create failed: material initialization failed.");
            return {};
        }

        mat->SetVertexShader(vs);
        mat->SetPixelShader(ps);
        mat->SetInputLayout(il);

        // Unlit表示用：BaseColorを白（テクスチャ色をそのまま使用）
        mat->GetParams().m_baseColor = Engine::Vector4(1, 1, 1, 1);

        // テクスチャ読み込み（Assets/Textures/UI/Title.png）
        // ロード失敗時はログを出して描画スキップ（クラッシュしない）
        Engine::TextureLoadOptions opt{};
        opt.m_generateMipMaps = true;
        opt.m_forceSRgb = true;

        auto tex = ctx.m_assets->LoadTexture(L"Textures/UI/TitleLogo.png", opt);
        if (tex && tex->IsLoaded()) {
            mat->SetTexture(tex);
            mat->GetParams().m_flags |= Engine::kMaterialFlagUseTexture;
            mat->EnableTexture(true);
            Engine::Logger::Info("TitleQuadMaterial: Title.png texture applied.");
        }
        else {
            // テクスチャが存在しない場合でもクラッシュしない
            mat->EnableTexture(false);
            Engine::Logger::Warn("TitleQuadMaterial: Title.png load failed (texture not found or invalid). Rendering will be skipped.");
        }

        return mat;
    }

} // namespace Game
