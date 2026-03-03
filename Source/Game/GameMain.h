#pragma once

#include <memory>

#include "Engine/Core/IGame.h"
#include "Engine/Resources/AssetManager.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/Scene/CameraSystem.h"
#include "Engine/Scene/LightSystem.h"
#include "Engine/UI/Canvas.h"
#include "Engine/Materials/MaterialLibrary.h"
#include "Engine/Graphics/FadeSystem.h"
#include "Engine/Debug/DebugVisualizationSystem.h"
#include "Engine/Debug/DebugImGuiSystem.h"

namespace Engine {
    class Application;
    class Scene;
    class Material;
}

namespace Game {

    class GameMain final : public Engine::IGame {
    public:

		/// @brief  ゲーム初期化
		/// @param  app Engine側アプリケーション（World/Assets等へアクセスする入口）
        /// @return 初期化成功ならtrue
        bool Initialize(Engine::Application& app) override;
		/// @brief  ゲーム終了処理（安全に複数回呼ばれても壊れない設計を推奨）
        void Finalize() override;

		/// @brief  毎フレーム更新
		/// @param  deltaTime 秒
        void Update(float deltaTime) override;

		/// @brief  毎フレーム描画（描画要求の送出）
        void Draw() override;

        /// @brief Canvas取得（Engine側からUI描画を呼び出すため）
        Engine::Canvas* GetCanvas() override { return &m_canvas; }

        /// @brief FadeSystem取得（Engine側からフェード描画を呼び出すため）
        Engine::FadeSystem* GetFadeSystem() override { return &m_fadeSystem; }

        /// @brief DebugVisualizationSystem取得（Engine側からデバッグ描画を呼び出すため）
        Engine::DebugVisualizationSystem* GetDebugVisualizationSystem() override { return &m_debugVisualization; }

        /// @brief DebugImGuiSystem取得（Engine側からImGuiデバッグUI描画を呼び出すため）
        Engine::DebugImGuiSystem* GetDebugImGuiSystem() override { return &m_debugImGui; }

        /// @brief LightSystem取得（Engine側からライト情報を取得するため）
        Engine::LightSystem* GetLightSystem() override { return &m_lightSystem; }



    private:
        Engine::Application* m_app = nullptr; // non-owning
        Engine::Scene* m_scene = nullptr;     // non-owning

        Engine::AssetManager m_assets;

        // Material資産の管理
        Engine::MaterialLibrary m_materialLibrary;

        // 共有Material（SampleWorldで直接使いたい場合にも渡せる）
        std::shared_ptr<Engine::Material> m_sharedMaterial;

        Engine::SceneManager m_sceneManager;

        // Camera管理システム
        Engine::CameraSystem m_cameraSystem;

        // Light管理システム
        Engine::LightSystem m_lightSystem;

        // Canvas UI管理システム
        Engine::Canvas m_canvas;

        // フェードシステム（シーン遷移・演出用）
        Engine::FadeSystem m_fadeSystem;

        // デバッグ可視化システム
        Engine::DebugVisualizationSystem m_debugVisualization;

        // デバッグImGuiシステム
        Engine::DebugImGuiSystem m_debugImGui;
    };

} // namespace Game
