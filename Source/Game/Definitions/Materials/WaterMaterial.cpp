#define NOMINMAX
#include "WaterMaterial.h"

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

    struct WaterVertexPosNormColorUv {
        Engine::Vector3 m_pos;
        Engine::Vector3 m_normal;
        Engine::Vector4 m_color;
        Engine::Vector2 m_uv;
    };

    static Engine::VertexInputLayout CreateWaterLayout() {
        Engine::VertexInputLayout layout;
        layout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, (UINT)offsetof(WaterVertexPosNormColorUv, m_pos),    D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layout.push_back({ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, (UINT)offsetof(WaterVertexPosNormColorUv, m_normal), D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layout.push_back({ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, (UINT)offsetof(WaterVertexPosNormColorUv, m_color),  D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, (UINT)offsetof(WaterVertexPosNormColorUv, m_uv),     D3D11_INPUT_PER_VERTEX_DATA, 0 });
        return layout;
    }

    std::shared_ptr<Engine::Material> WaterMaterial::Create(const Engine::MaterialBuildContext& ctx) {
        if (!ctx.m_device || !ctx.m_assets) {
            Engine::Logger::Error("WaterMaterial::Create failed: ctx invalid.");
            return {};
        }

        // WaterVS/WaterPSを使用
        auto vs = ctx.m_assets->LoadVertexShader(L"Shaders/WaterVS.cso");
        auto ps = ctx.m_assets->LoadPixelShader(L"Shaders/WaterPS.cso");
        if (!vs || !ps) {
            Engine::Logger::Error("WaterMaterial::Create failed: shader load failed.");
            return {};
        }

        auto il = ctx.m_assets->CreateInputLayout(L"WaterLayout", CreateWaterLayout(), *vs);
        if (!il) {
            Engine::Logger::Error("WaterMaterial::Create failed: input layout creation failed.");
            return {};
        }

        auto mat = std::make_shared<Engine::Material>();
        if (!mat->Initialize(ctx.m_device)) {
            Engine::Logger::Error("WaterMaterial::Create failed: material initialization failed.");
            return {};
        }

        mat->SetVertexShader(vs);
        mat->SetPixelShader(ps);
        mat->SetInputLayout(il);

        // 水面の基本色（青緑系）
        mat->GetParams().m_baseColor = Engine::Vector4(0.3f, 0.6f, 0.8f, 0.7f);

        // テクスチャは不要（反射テクスチャはシェーダー側で別途バインド）
        mat->EnableTexture(false);

        Engine::Logger::Info("WaterMaterial: Water material created.");
        return mat;
    }

} // namespace Game
