/// @file   FreeLookCameraController.h
/// @brief  右クリック＋マウスドラッグでカメラの視点を回転させるコンポーネント
/// @note   Prefab化したCameraにAddComponentして使用することを想定
#pragma once

#include "Engine/Scene/Component.h"

namespace Engine {
    class Transform;
}

namespace Game {

    /// @brief 自由視点カメラコントローラーの設定
    struct FreeLookCameraSettings final {
        float m_mouseSensitivity = 0.3f;   ///< マウス感度（度/ピクセル）
        float m_pitchLimitMin = -89.0f;    ///< ピッチ角度下限（度）
        float m_pitchLimitMax = 89.0f;     ///< ピッチ角度上限（度）
    };

    /// @brief 右クリック＋マウスドラッグでカメラの視点を回転させるコンポーネント
    /// @note  右クリック押下中にのみマウスドラッグでカメラの回転・向きを変更できる
    ///        Prefab化したCameraにAddComponentして使用可能
    class FreeLookCameraController final : public Engine::Component {
    public:
        /// @brief コンストラクタ
        /// @param mouseSensitivity マウス感度（度/ピクセル、デフォルト: 0.3）
        explicit FreeLookCameraController(float mouseSensitivity = 0.3f);

        ~FreeLookCameraController() override = default;

        //============================================================
        // ライフサイクル
        //============================================================

        /// @brief Start時に呼ばれる（初期回転角度を取得）
        void OnStart() override;

        /// @brief 毎フレーム更新
        /// @param deltaTime フレーム経過時間
        void Update(float deltaTime) override;

        //============================================================
        // 設定
        //============================================================

        /// @brief  設定を取得
        /// @return カメラ設定への参照
        [[nodiscard]] FreeLookCameraSettings& GetSettings();

        /// @brief  設定を取得（const版）
        /// @return カメラ設定へのconst参照
        [[nodiscard]] const FreeLookCameraSettings& GetSettings() const;

        /// @brief  マウス感度を設定
        /// @param  sensitivity 感度値（度/ピクセル）
        void SetMouseSensitivity(float sensitivity);

        /// @brief  マウス感度を取得
        /// @return マウス感度
        [[nodiscard]] float GetMouseSensitivity() const;

        /// @brief  ピッチ角度制限を設定
        /// @param  minPitch 下限角度（度）
        /// @param  maxPitch 上限角度（度）
        void SetPitchLimits(float minPitch, float maxPitch);

        //============================================================
        // 状態クエリ
        //============================================================

        /// @brief  現在カメラ操作中か確認（右クリック中）
        /// @return 操作中ならtrue
        [[nodiscard]] bool IsLooking() const;

        /// @brief  現在のYaw角度を取得
        /// @return Yaw角度（度）
        [[nodiscard]] float GetCurrentYaw() const;

        /// @brief  現在のPitch角度を取得
        /// @return Pitch角度（度）
        [[nodiscard]] float GetCurrentPitch() const;

    private:
        Engine::Transform* m_transform = nullptr;   ///< 所有者のTransform（キャッシュ）
        FreeLookCameraSettings m_settings;          ///< カメラ設定

        bool m_isLooking = false;                   ///< 現在操作中フラグ
        float m_yaw = 0.0f;                         ///< 現在のYaw角度（度）
        float m_pitch = 0.0f;                       ///< 現在のPitch角度（度）
    };

} // namespace Game
