#include "SceneManager.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/CameraSystem.h"
#include "Engine/Scene/LightSystem.h"
#include "Engine/UI/Canvas.h"

namespace Engine {

    bool SceneManager::Initialize(SceneContext ctx, std::unique_ptr<SceneBase> firstScene) {
        m_ctx = ctx;
        m_current = std::move(firstScene);
        m_next.reset();

        m_isInitialized = true;

        if (m_current) {
            m_current->Initialize(m_ctx);
        }

		return true;
    }

    void SceneManager::Finalize() {
        if (!m_isInitialized) return;

        if (m_current) {
            m_current->Finalize(m_ctx);
            m_current.reset();
        }
        m_next.reset();

        // === アプリケーション終了時のクリーンアップ処理 ===
        // シーン遷移時と同様に共有リソースをリセットする

        // Scene（GameObject管理）をリセット
        if (m_ctx.m_scene) {
            m_ctx.m_scene->Reset();
        }

        // CameraSystemのメインカメラ参照をクリア
        if (m_ctx.m_cameraSystem) {
            m_ctx.m_cameraSystem->SetMainCamera(nullptr);
        }

        // LightSystemのライトをクリア
        if (m_ctx.m_lightSystem) {
            m_ctx.m_lightSystem->ClearLights();
        }

        // CanvasのUI要素をクリア
        if (m_ctx.m_canvas) {
            m_ctx.m_canvas->ClearElements();
            m_ctx.m_canvas->ClearUIObjects();
        }

        m_isInitialized = false;
    }

    void SceneManager::ChangeScene(std::unique_ptr<SceneBase> nextScene) {
        m_next = std::move(nextScene);
    }

    void SceneManager::ApplyPendingSceneIfNeeded() {
        if (!m_next) return;

        // 旧Sceneの終了処理
        if (m_current) {
            m_current->Finalize(m_ctx);
        }

        // === シーン遷移時のクリーンアップ処理 ===
        // シーン遷移時に共有リソースを完全にリセットする

        // 1. Scene（GameObject管理）をリセット
        //    これにより旧Sceneの全GameObjectが破棄される
        if (m_ctx.m_scene) {
            m_ctx.m_scene->Reset();
            Logger::Info("SceneManager: Scene reset completed.");
        }

        // 2. CameraSystemのメインカメラ参照をクリア
        //    カメラGameObjectは上記Resetで破棄されているため、
        //    ダングリングポインタを防ぐ
        if (m_ctx.m_cameraSystem) {
            m_ctx.m_cameraSystem->SetMainCamera(nullptr);
            Logger::Info("SceneManager: CameraSystem main camera cleared.");
        }

        // 3. LightSystemのライトをクリア
        //    各SceneBaseで設定されたライトは次のSceneには不要
        if (m_ctx.m_lightSystem) {
            m_ctx.m_lightSystem->ClearLights();
            Logger::Info("SceneManager: LightSystem lights cleared.");
        }

        // 4. CanvasのUI要素をクリア
        //    Legacy UI要素とUI GameObjectの両方をクリア
        if (m_ctx.m_canvas) {
            m_ctx.m_canvas->ClearElements();
            m_ctx.m_canvas->ClearUIObjects();
            Logger::Info("SceneManager: Canvas UI elements cleared.");
        }

        // 新Sceneに切り替え
        m_current = std::move(m_next);

        // 新Sceneの開始処理
        if (m_current) {
            m_current->Initialize(m_ctx);
        }
    }

    void SceneManager::Update(float deltaTime) {
        if (!m_isInitialized) return;

        // フレームの先頭で切替（安全）
        ApplyPendingSceneIfNeeded();

        if (m_current) {
            m_current->Update(m_ctx, deltaTime);
        }
    }

    void SceneManager::Render() {
        if (!m_isInitialized) return;

        if (m_current) {
            m_current->Render(m_ctx);
        }
    }

} // namespace Engine
