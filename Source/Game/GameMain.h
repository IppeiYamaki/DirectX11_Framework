#pragma once

#include <memory>

#include "Engine/Core/IGame.h"
#include "Engine/Resources/AssetManager.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/Scene/CameraSystem.h"
#include "Engine/Scene/LightSystem.h"
#include "Engine/UI/Canvas.h"
#include "Materials/MaterialLibrary.h"

namespace Engine {
    class Application;
    class Scene;
    class Material;
}

namespace Game {

    class GameMain final : public Engine::IGame {
    public:
        bool Initialize(Engine::Application& app) override;
        void Finalize() override;

        void Update(float deltaTime) override;
        void Draw() override;

    private:
        Engine::Application* m_app = nullptr; // non-owning
        Engine::Scene* m_scene = nullptr;     // non-owning

        Engine::AssetManager m_assets;

        // Material資産の管理
        Game::MaterialLibrary m_materialLibrary;

        // 共有Material（SampleWorldで直接使いたい場合にも渡せる）
        std::shared_ptr<Engine::Material> m_sharedMaterial;

        Engine::SceneManager m_sceneManager;

        // Camera管理システム
        Engine::CameraSystem m_cameraSystem;

        // Light管理システム
        Engine::LightSystem m_lightSystem;

        // Canvas UI管理システム
        Engine::Canvas m_canvas;
    };

} // namespace Game
