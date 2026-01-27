#define NOMINMAX
#include "GameMain.h"

#include "Engine/Core/Logger.h"
#include "Engine/Core/Application.h"

#include "Engine/Graphics/GraphicsDevice.h"
#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Graphics/Material.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Math/Vector4.h"

#include "Engine/Scene/SceneContext.h"
#include "Game/Scenes/SampleScene.h"

// Material資産
#include "Materials/MaterialBuildContext.h"
#include "Materials/SampleCubeMaterial.h"

namespace Game {

    bool GameMain::Initialize(Engine::Application& app) {
        Engine::Logger::Info("GameMain Initialize");

        m_app = &app;
        m_scene = app.GetScene();
        if (!m_scene) return false;

        auto* gd = app.GetGraphicsDevice();
        auto* rs = app.GetRenderSystem();
        if (!gd || !rs) return false;

        //========================
        // AssetManager
        //========================
        if (!m_assets.Initialize(gd->GetDevice())) return false;
        m_assets.SetBaseDirectory(L"Assets");

        //========================
        // MaterialLibrary（Material生成からShader / InputLayoutまで責務を持つ）
        //========================
        {
            MaterialBuildContext mbc{};
            mbc.m_device = gd->GetDevice();
            mbc.m_assets = &m_assets;

            m_materialLibrary.Initialize(mbc);

            // 「Assetsで作ったMaterial」を起動時に生成しキャッシュ（プレウォーム）
            m_sharedMaterial = m_materialLibrary.GetOrCreate<SampleCubeMaterial>();
            if (!m_sharedMaterial) return false;
        }


        //========================
        // SceneContext
        //========================
        Engine::SceneContext ctx{};
        ctx.m_app = m_app;
        ctx.m_scene = m_scene;
        ctx.m_renderSystem = rs;

        // 一部依存：借用D3Dデバイスを渡す
        ctx.m_device = gd->GetDevice();

        // 任意：Sceneが必要なら参照できるように渡す
        ctx.m_assets = &m_assets;

        // 共有Material（SampleSceneが直接使う場合）
        ctx.m_sharedMaterial = m_sharedMaterial;

        // PrefabがMaterialLibraryを使うために渡す
        ctx.m_materials = &m_materialLibrary;

        // PrefabManager（Prefabのキャッシュ）
        ctx.m_prefabs = &m_prefabManager;

        // 最初の空間はSampleScene
        m_sceneManager.Initialize(ctx, std::make_unique<Game::SampleScene>());

        return true;
    }

    void GameMain::Finalize() {
        Engine::Logger::Info("GameMain Finalize");


        m_sceneManager.Finalize();
        m_prefabManager.Clear();
        m_sharedMaterial.reset();
        m_materialLibrary.Finalize();
        m_assets.Finalize();

        m_scene = nullptr;
        m_app = nullptr;

    }

    void GameMain::Update(float deltaTime) {
        // Scene（状態）更新（遷移予約など）
        m_sceneManager.Update(deltaTime);

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
    }

} // namespace Game
