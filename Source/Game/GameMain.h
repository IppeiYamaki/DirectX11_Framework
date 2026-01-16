#pragma once

#include <memory>

#include "Engine/Core/IGame.h"
#include "Engine/Resources/AssetManager.h"
#include "Engine/Graphics/Mesh.h"
#include "Engine/Graphics/RenderSystem.h"

namespace Engine {
    class Application;
    class World;
    class VertexShader;
    class PixelShader;
    class InputLayout;
}


namespace Game {

    class GameMain final : public Engine::IGame {
    public:
        GameMain() = default;
        ~GameMain() override = default;

        bool Initialize(Engine::Application& app) override;
        void Finalize() override;

        void Update(float deltaTime) override;
        void Draw() override;

    private:
		// Engine参照
        Engine::Application*                    m_app       = nullptr;  //　アプリケーション（借用）
        Engine::World*                          m_world     = nullptr;  // シーン管理（借用）

		Engine::AssetManager                    m_assets;               // アセット管理

		std::shared_ptr<Engine::VertexShader>   m_vs;                   // バーテックスシェーダ
		std::shared_ptr<Engine::PixelShader>    m_ps;                   // ピクセルシェーダ
		std::shared_ptr<Engine::InputLayout>    m_il;                   // 入力レイアウト

        Engine::Mesh                            m_mesh;

        Engine::RenderItem                      m_triangleItem;
    };

} // namespace Game
