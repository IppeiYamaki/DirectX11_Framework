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

    class Scene;

	/// @brief アプリケーションの中核を担うクラス
	/// @brief 各種サブシステムの生成・管理、メインループの実行を担当する
    class Application final {
    public:
		// ============================================================
		// Constructor / Destructor
		// ============================================================
        Application() = default;
        ~Application();

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        //============================================================
        // Lifecycle
        //============================================================

		/// @brief アプリケーションを初期化する
		/// @brief 内部で各種サブシステムを生成・初期化する
		/// @param settings アプリケーション設定
		/// @return 初期化に成功した場合 true を返す
        bool Initialize(const ApplicationSettings& settings);
		/// @brief アプリケーションを終了する
		/// @brief 内部で生成した各種サブシステムを破棄する
        void Finalize();
		/// @brief アプリケーションをリセットする
		/// @brief 内部で生成した各種サブシステムを再初期化する
        void Reset();

        //============================================================
        // Main loop
        //============================================================

		/// @brief メインループを実行する
		/// @param game ゲーム側システムインスタンス
		/// @return 終了コード（通常は0）
        int Run(IGame& game);

        //============================================================
        // Control
        //============================================================

		/// @brief アプリケーション終了を要求する
		/// @brief Run() 内でこの関数が呼ばれた場合、メインループを抜けて終了処理に移行する
        void RequestQuit();
		/// @brief アプリケーション終了が要求されているか確認する
		/// @return 終了要求がされていれば true を返す
        bool IsQuitRequested() const;
		/// @brief アプリケーションが初期化されているか確認する
		/// @return 初期化されていれば true を返す
        bool IsInitialized() const;

        //============================================================
		// Getters (const / non-const)
        //============================================================

		/// @brief  アプリケーション設定を取得
		/// @return アプリケーション設定へのconst参照
        const ApplicationSettings& GetSettings() const;

		/// @brief  時間管理システムを取得
        /// @return 時間管理システムへの参照
        const Time& GetTime() const;

		/// @brief  ワールドを取得
		/// @return ワールドへの参照
              World* GetWorld();
        const World* GetWorld() const;

        /// @brief  ウィンドウを取得
		/// @return ウィンドウへの参照
              Window* GetWindow();
        const Window* GetWindow() const;

		/// @brief  グラフィックスデバイスを取得
		/// @return グラフィックスデバイスへの参照
              GraphicsDevice* GetGraphicsDevice();
        const GraphicsDevice* GetGraphicsDevice() const;

		/// @brief  レンダーシステムを取得
		/// @return レンダーシステムへの参照
              RenderSystem* GetRenderSystem();
        const RenderSystem* GetRenderSystem() const;

		/// @brief  シーンを取得
		/// @return シーンへの参照
              Scene* GetScene();
        const Scene* GetScene() const;

    private:
		bool                            m_isInitialized     = false;    // アプリケーションが初期化されているか
		bool                            m_isQuitRequested	= false;    // アプリケーション終了が要求されているか

		ApplicationSettings             m_settings{};                   // アプリケーション設定
		Time                            m_time{};                       // 時間管理システム

		std::unique_ptr<Window>         m_window;                       // ウィンドウ
		std::unique_ptr<GraphicsDevice> m_graphicsDevice;               // グラフィックスデバイス
		std::unique_ptr<RenderSystem>   m_renderSystem;                 // レンダーシステム
		std::unique_ptr<World>          m_world;                        // ワールド
    };

} // namespace Engine
