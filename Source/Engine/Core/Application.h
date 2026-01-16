#pragma once

#include <memory>

#include "Engine/Core/ApplicationSettings.h"
#include "Engine/Core/Time.h"
#include "Engine/Core/IGame.h"

#include "Engine/Platform/Window.h"
#include "Engine/Graphics/GraphicsDevice.h"
#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Scene/World.h"

namespace Engine {

    /**
     * @brief Engine の司令塔（初期化順序 + メインループ固定）
     *
     * - Window / GraphicsDevice / RenderSystem / World / Time を所有する
     * - ループはここで回す（WinMain を薄くする）
     */
    class Application final {
    public:
        Application() = default;
        ~Application(); // unique_ptr のため cpp で定義（前方宣言型の完全型が必要）

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        //============================================================
        // Lifecycle
        //============================================================
        bool Initialize(const ApplicationSettings& settings);
        void Finalize();
        void Reset();

        //============================================================
        // Main loop
        //============================================================
        int Run(IGame& game);

        //============================================================
        // Control
        //============================================================
        void RequestQuit();
        bool IsQuitRequested() const;
        bool IsInitialized() const;

        //============================================================
        // Getters（非const参照Getは原則禁止 → ポインタで返す）
        //============================================================
        const ApplicationSettings& GetSettings() const;

        const Time& GetTime() const;

              World* GetWorld();
        const World* GetWorld() const;

        // Engine内部用（ゲームからは触らない想定。必要なら公開範囲を見直す）
              Window* GetWindow();
        const Window* GetWindow() const;

              GraphicsDevice* GetGraphicsDevice();
        const GraphicsDevice* GetGraphicsDevice() const;

              RenderSystem* GetRenderSystem();
        const RenderSystem* GetRenderSystem() const;

    private:
		bool                            m_isInitialized     = false;    // 初期化済みフラグ
        bool                            m_isQuitRequested   = false;    // 終了要求フラグ

		ApplicationSettings             m_settings{};                   // アプリケーション設定
		Time                            m_time{};                       // 時間管理

		std::unique_ptr<Window>         m_window;                       // ウィンドウ管理
		std::unique_ptr<GraphicsDevice> m_graphicsDevice;               // グラフィックスデバイス管理
		std::unique_ptr<RenderSystem>   m_renderSystem;                 // レンダリング管理
		std::unique_ptr<World>          m_world;                        // シーン管理
    };

} // namespace Engine
