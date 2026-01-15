#include "GameMain.h"

#include <cstddef>
#include <DirectXMath.h>

#include "Engine/Core/Logger.h"
#include "Engine/Core/Application.h"
#include "Engine/Graphics/GraphicsDevice.h"
#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Scene/World.h"
#include "Engine/Graphics/Shader.h"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"

namespace Game {

#include "Engine/Math/Vector2.h"

    struct VertexPosNormColorUv {
        Engine::Vector3 m_pos;
        Engine::Vector3 m_normal;
        Engine::Vector4 m_color;
        Engine::Vector2 m_uv;
    };


    static Engine::VertexInputLayout CreatePosNormColorUvLayout() {
        Engine::VertexInputLayout layout;
        layout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, (UINT)offsetof(VertexPosNormColorUv, m_pos),    D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layout.push_back({ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, (UINT)offsetof(VertexPosNormColorUv, m_normal), D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layout.push_back({ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, (UINT)offsetof(VertexPosNormColorUv, m_color),  D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, (UINT)offsetof(VertexPosNormColorUv, m_uv),     D3D11_INPUT_PER_VERTEX_DATA, 0 });
        return layout;
    }

    bool GameMain::Initialize(Engine::Application& app) {
        Engine::Logger::Info("GameMain Initialize (Triangle)");

        m_app = &app;
        m_world = app.GetWorld();
        if (m_world == nullptr) return false;

        auto* gd = app.GetGraphicsDevice();
        if (gd == nullptr) return false;

        // AssetManager 初期化
        if (!m_assets.Initialize(gd->GetDevice())) return false;
        m_assets.SetBaseDirectory(L"Assets");

        // Shader 読み込み（Assets/Shaders に cso がある想定）
        m_vs = m_assets.LoadVertexShader(L"Shaders/DefaultVS.cso");
        m_ps = m_assets.LoadPixelShader(L"Shaders/DefaultPS.cso");
        if (!m_vs || !m_ps) return false;

        // InputLayout
        m_il = m_assets.CreateInputLayout(L"PosColor", CreatePosNormColorUvLayout(), *m_vs);
        if (!m_il) return false;

        // 三角形（クリップ空間）
        const VertexPosNormColorUv vertices[] = {
            { Engine::Vector3(0.0f,  0.5f, 0.0f), Engine::Vector3(0,0,-1), Engine::Vector4(1,0,0,1), Engine::Vector2(0.5f, 0.0f) },
            { Engine::Vector3(0.5f, -0.5f, 0.0f), Engine::Vector3(0,0,-1), Engine::Vector4(0,1,0,1), Engine::Vector2(1.0f, 1.0f) },
            { Engine::Vector3(-0.5f, -0.5f, 0.0f), Engine::Vector3(0,0,-1), Engine::Vector4(0,0,1,1), Engine::Vector2(0.0f, 1.0f) },
        };
        const std::uint32_t indices[] = { 0, 1, 2 };


        if (!m_mesh.Create(gd->GetDevice(), vertices, sizeof(VertexPosNormColorUv), 3, indices, 3)) {
            return false;
        }

        return true;
    }

    void GameMain::Finalize() {
        Engine::Logger::Info("GameMain Finalize (Triangle)");

        m_mesh.Destroy();
        m_il.reset();
        m_ps.reset();
        m_vs.reset();

        m_assets.Finalize();

        m_world = nullptr;
        m_app = nullptr;
    }

    void GameMain::Update(float deltaTime) {
        (void)deltaTime;
    }

    void GameMain::Draw() {
        // RenderSystem に “デバッグ描画データ” を渡す（今はここが橋渡し役）
        auto* rs = m_app->GetRenderSystem();
        if (!rs) return;

        rs->SetDebugDraw(
            &m_mesh,
            m_il->GetInputLayout(),
            m_vs->GetShader(),
            m_ps->GetShader()
        );
    }

} // namespace Game
