#define NOMINMAX
#include "GameMain.h"

#include "Engine/Core/Logger.h"
#include "Engine/Core/Application.h"
#include "Engine/Core/ApplicationSettings.h"

#include "Engine/Graphics/GraphicsDevice.h"
#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Graphics/Material.h"

#include "Engine/Platform/Window.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Math/Vector4.h"

#include "Engine/Scene/SceneContext.h"
#include "Game/Scenes/SampleScene.h"
#include "Game/Scenes/GameScene.h"
#include "Game/Scenes/TitleScene.h"
#include "Game/Scenes/MazeCollectScene.h"

// Material資産
#include "Engine/Materials/MaterialBuildContext.h"
#include "Game/Definitions/Materials/SampleCubeMaterial.h"

namespace Game {

    bool GameMain::Initialize(Engine::Application& app) {
        Engine::Logger::Info("GameMain Initialize");

        m_app = &app;
        m_scene = app.GetScene();
        if (!m_scene) return false;

        auto* gd = app.GetGraphicsDevice();
        auto* rs = app.GetRenderSystem();
        auto* wnd = app.GetWindow();
        if (!gd || !rs || !wnd) return false;

        //========================
        // AssetManager
        //========================
        if (!m_assets.Initialize(gd->GetDevice())) return false;
        m_assets.SetBaseDirectory(L"Assets");

        //========================
        // MaterialLibrary（Material生成からShader / InputLayoutまで責務を持つ）
        //========================
        {
            Engine::MaterialBuildContext mbc{};
            mbc.m_device = gd->GetDevice();
            mbc.m_assets = &m_assets;

            m_materialLibrary.Initialize(mbc);

            // 「Assetsで作ったMaterial」を起動時に生成しキャッシュ（プレウォーム）
            m_sharedMaterial = m_materialLibrary.GetOrCreate<SampleCubeMaterial>();
            if (!m_sharedMaterial) return false;
        }

        //========================
        // CameraSystem初期化
        //========================
        if (!m_cameraSystem.Initialize(m_scene, rs)) {
            Engine::Logger::Error("GameMain::Initialize failed: CameraSystem initialize failed.");
            return false;
        }

        //========================
        // LightSystem初期化
        //========================
        if (!m_lightSystem.Initialize(rs)) {
            Engine::Logger::Error("GameMain::Initialize failed: LightSystem initialize failed.");
            return false;
        }

        //========================
        // Canvas初期化
        //========================
        const auto& settings = app.GetSettings();
        if (!m_canvas.Initialize(static_cast<float>(settings.m_width), static_cast<float>(settings.m_height))) {
            Engine::Logger::Error("GameMain::Initialize failed: Canvas initialize failed.");
            return false;
        }

        //========================
        // FadeSystem初期化
        //========================
        if (!m_fadeSystem.Initialize(rs, static_cast<float>(settings.m_width), static_cast<float>(settings.m_height))) {
            Engine::Logger::Error("GameMain::Initialize failed: FadeSystem initialize failed.");
            return false;
        }

        //========================
        // DebugVisualizationSystem初期化
        //========================
        if (!m_debugVisualization.Initialize(gd, rs)) {
            // デバッグ機能は必須ではないため警告のみ
            Engine::Logger::Warn("GameMain::Initialize: DebugVisualizationSystem initialize failed (debug features disabled).");
        }

        //========================
        // DebugImGuiSystem初期化
        //========================
        if (!m_debugImGui.Initialize(wnd->GetHwnd(), gd->GetDevice(), gd->GetContext())) {
            // デバッグ機能は必須ではないため警告のみ
            Engine::Logger::Warn("GameMain::Initialize: DebugImGuiSystem initialize failed (debug UI disabled).");
        }

        //========================
        // SceneContext
        //========================
        Engine::SceneContext ctx{};
        ctx.m_app = m_app;
        ctx.m_scene = m_scene;
        ctx.m_renderSystem = rs;
        ctx.m_cameraSystem = &m_cameraSystem;
        ctx.m_lightSystem = &m_lightSystem;
        ctx.m_canvas = &m_canvas;
        ctx.m_fadeSystem = &m_fadeSystem;

        // D3Dデバイスを渡す
        ctx.m_device = gd->GetDevice();

        // Sceneが必要なら参照できるように渡す
        ctx.m_assets = &m_assets;

        // 共有Material（SampleSceneが直接使う場合）
        ctx.m_sharedMaterial = m_sharedMaterial;

        // PrefabがMaterialLibraryを使うために渡す
        ctx.m_materials = &m_materialLibrary;

        // Scene遷移のためにSceneManagerを渡す
        ctx.m_sceneManager = &m_sceneManager;

        // 最初の空間 - TitleSceneを起動
        m_sceneManager.Initialize(ctx, std::make_unique<Game::TitleScene>());

        return true;
    }

    void GameMain::Finalize() {
        Engine::Logger::Info("GameMain Finalize");

        m_sceneManager.Finalize();
        m_debugImGui.Finalize();
        m_debugVisualization.Finalize();
        m_fadeSystem.Finalize();
        m_canvas.Finalize();
        m_lightSystem.Finalize();
        m_cameraSystem.Finalize();
        m_sharedMaterial.reset();
        m_materialLibrary.Finalize();
        m_assets.Finalize();

        m_scene = nullptr;
        m_app = nullptr;
    }

    void GameMain::Update(float deltaTime) {
        // Scene（状態）更新（遷移予約など）
        m_sceneManager.Update(deltaTime);

        // CameraSystem更新
        m_cameraSystem.Update(deltaTime);

        // LightSystem更新
        m_lightSystem.Update(deltaTime);
        m_lightSystem.ApplyToRenderSystem();

        // Canvas更新
        m_canvas.Update(deltaTime);

        // FadeSystem更新
        m_fadeSystem.Update(deltaTime);

        // DebugVisualizationSystem更新（キー入力チェック）
        // SceneContextを構築してデバッグライトマーカーの生成/削除に使用
        {
            Engine::SceneContext ctx{};
            ctx.m_app = m_app;
            ctx.m_scene = m_scene;
            ctx.m_renderSystem = m_app ? m_app->GetRenderSystem() : nullptr;
            ctx.m_cameraSystem = &m_cameraSystem;
            ctx.m_lightSystem = &m_lightSystem;
            ctx.m_canvas = &m_canvas;
            ctx.m_fadeSystem = &m_fadeSystem;
            ctx.m_device = m_app && m_app->GetGraphicsDevice() ? m_app->GetGraphicsDevice()->GetDevice() : nullptr;
            ctx.m_assets = &m_assets;
            ctx.m_sharedMaterial = m_sharedMaterial;
            ctx.m_materials = &m_materialLibrary;
            ctx.m_sceneManager = &m_sceneManager;
            
            m_debugVisualization.Update(deltaTime, ctx, &m_lightSystem);
        }

        // DebugImGuiSystemをDebugVisualizationSystemと同期
        // デバッグ可視化が有効になったらImGuiも有効化
        if (m_debugVisualization.IsEnabled() != m_debugImGui.IsEnabled()) {
            m_debugImGui.SetEnabled(m_debugVisualization.IsEnabled());
        }

        // Scene更新（Component処理）
        if (m_scene) {
            m_scene->Update(deltaTime);
            m_scene->LateUpdate(deltaTime);
        }
    }

    void GameMain::Draw() {
        // Scene固有UIなど
        m_sceneManager.Render();

        // Scene::Draw -> MeshRenderer::Draw -> RenderSystem に RenderItem を積む
        if (m_scene) {
            m_scene->Draw();
        }

        // デバッグ可視化描画はRenderSystem::Draw()内で3Dオブジェクト描画後、UI描画前に実行される
        // （GetDebugVisualizationSystem()経由でRenderSystemに渡される）
        // Canvas描画はRenderSystem::Draw()内でデバッグ描画後、Present直前に実行される
        // （GetCanvas()経由でRenderSystemに渡される）
        // FadeSystem描画もRenderSystem::Draw()内でCanvas描画後に実行される
        // （GetFadeSystem()経由でRenderSystemに渡される）
        // ImGuiデバッグUIはRenderSystem::Draw()内でFadeSystem描画後に実行される
        // （GetDebugImGuiSystem()経由でRenderSystemに渡される）
    }

} // namespace Game
