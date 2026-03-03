#define NOMINMAX
#include "ParticleMaterial.h"

#include <cstddef>

#include "Engine/Core/Logger.h"
#include "Engine/Resources/AssetManager.h"

#include "Engine/Graphics/Material.h"
#include "Engine/Graphics/Shader.h"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"

#include "Engine/Materials/MaterialBuildContext.h"

namespace Engine {

    struct VertexPosNormColorUv {
        Engine::Vector3 m_pos;
        Engine::Vector3 m_normal;
        Engine::Vector4 m_color;
        Engine::Vector2 m_uv;
    };

    static Engine::VertexInputLayout CreateParticleLayout() {
        Engine::VertexInputLayout layout;
        layout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, (UINT)offsetof(VertexPosNormColorUv, m_pos),    D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layout.push_back({ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, (UINT)offsetof(VertexPosNormColorUv, m_normal), D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layout.push_back({ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, (UINT)offsetof(VertexPosNormColorUv, m_color),  D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, (UINT)offsetof(VertexPosNormColorUv, m_uv),     D3D11_INPUT_PER_VERTEX_DATA, 0 });
        return layout;
    }

    std::shared_ptr<Engine::Material> ParticleMaterial::Create(const Engine::MaterialBuildContext& ctx) {
        if (!ctx.m_device || !ctx.m_assets) {
            Engine::Logger::Error("ParticleMaterial::Create failed: ctx invalid.");
            return {};
        }

        auto vs = ctx.m_assets->LoadVertexShader(L"Shaders/ParticleVS.cso");
        auto ps = ctx.m_assets->LoadPixelShader(L"Shaders/ParticlePS.cso");
        if (!vs || !ps) {
            Engine::Logger::Error("ParticleMaterial::Create failed: Could not load shaders.");
            return {};
        }

        auto il = ctx.m_assets->CreateInputLayout(L"ParticleLayout", CreateParticleLayout(), *vs);
        if (!il) {
            Engine::Logger::Error("ParticleMaterial::Create failed: Could not create input layout.");
            return {};
        }

        auto mat = std::make_shared<Engine::Material>();
        if (!mat->Initialize(ctx.m_device)) {
            Engine::Logger::Error("ParticleMaterial::Create failed: Could not initialize material.");
            return {};
        }

        mat->SetVertexShader(vs);
        mat->SetPixelShader(ps);
        mat->SetInputLayout(il);

        // Set base color to white (color comes from vertex color)
        mat->GetParams().m_baseColor = Engine::Vector4(1, 1, 1, 1);
        
        // Disable texture by default (we'll use procedural circular shape in shader)
        mat->EnableTexture(false);

        Engine::Logger::Info("ParticleMaterial: Created successfully.");

        return mat;
    }

} // namespace Engine
