#pragma once

namespace Engine {

    class Application;
    class Canvas;
    class FadeSystem;
    class DebugVisualizationSystem;
    class DebugImGuiSystem;
    class LightSystem;

    /// @brief Game側（GameSystem）と Engine側（Application）を分離するためのインターフェース
    /// @brief Engine -> Game への呼出
	/// @brief Game   -> Application を通じて Engine の機能にアクセスする
    class IGame {
    public:
        virtual ~IGame() = default;

		/// @brief ゲーム初期化
		/// @param app Engine側アプリケーション（World/Assetsへアクセスする入口）
        /// @return 成功なら true
        virtual bool Initialize(Application& app) = 0;

		/// @brief ゲーム終了
        virtual void Finalize() = 0;

		/// @brief 毎フレーム更新
        /// @param deltaTime 秒
        virtual void Update(float deltaTime) = 0;

		/// @brief 毎フレーム描画
        virtual void Draw() = 0;

        /// @brief Canvasを取得（UI描画のため）
        /// @return Canvas（未使用の場合はnullptr）
        virtual Canvas* GetCanvas() { return nullptr; }

        /// @brief FadeSystemを取得（フェード描画のため）
        /// @return FadeSystem（未使用の場合はnullptr）
        virtual FadeSystem* GetFadeSystem() { return nullptr; }

        /// @brief DebugVisualizationSystemを取得（デバッグ描画のため）
        /// @return DebugVisualizationSystem（未使用の場合はnullptr）
        virtual DebugVisualizationSystem* GetDebugVisualizationSystem() { return nullptr; }

        /// @brief DebugImGuiSystemを取得（ImGuiデバッグUI描画のため）
        /// @return DebugImGuiSystem（未使用の場合はnullptr）
        virtual DebugImGuiSystem* GetDebugImGuiSystem() { return nullptr; }

        /// @brief LightSystemを取得（デバッグ描画のため）
        /// @return LightSystem（未使用の場合はnullptr）
        virtual LightSystem* GetLightSystem() { return nullptr; }
    };

} // namespace Engine
