/// @file   SelectUI.h
/// @brief  マウスクリックによるUI選択を処理するコンポーネント
#pragma once

#include "Engine/Scene/Component.h"

namespace Engine {
    class CameraComponent;
    class Canvas;
    class Scene;
}

namespace Game {

    /// @brief マウスクリックイベントを監視し、UI要素の選択を処理するコンポーネント
    /// @note  クリック位置からレイを生成し、UIとの当たり判定を行う
    class SelectUI : public Engine::Component {
    public:
        SelectUI() = default;
        ~SelectUI() override = default;

        //============================================================
        // Lifecycle
        //============================================================

        /// @brief 開始時に呼ばれる
        void OnStart() override;

        /// @brief 毎フレーム更新
        /// @param deltaTime フレーム経過時間
        void Update(float deltaTime) override;

        //============================================================
        // Configuration
        //============================================================

        /// @brief UIを配置しているCanvasを設定
        /// @param canvas Canvas参照
        void SetCanvas(Engine::Canvas* canvas);

        /// @brief 使用するカメラを設定
        /// @param camera Camera参照
        void SetCamera(Engine::CameraComponent* camera);

        /// @brief スクリーンサイズを設定
        /// @param width スクリーン幅
        /// @param height スクリーン高さ
        void SetScreenSize(float width, float height);

    private:
        Engine::Canvas* m_canvas = nullptr;     ///< UI要素を含むCanvas
        Engine::CameraComponent* m_camera = nullptr;     ///< レイ生成に使用するカメラ
        float m_screenWidth = 1280.0f;          ///< スクリーン幅
        float m_screenHeight = 720.0f;          ///< スクリーン高さ
    };

} // namespace Game
