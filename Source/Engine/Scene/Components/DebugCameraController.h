/// @file   DebugCameraController.h
/// @brief  デバッグモード時のフリールックカメラコントローラー
/// @note   右クリック＋マウスドラッグで視点回転、WASD/QEで移動
#pragma once

#include "Engine/Scene/Component.h"
#include "Engine/Math/Vector3.h"

namespace Engine {

    class Transform;

    /// @brief デバッグカメラ操作設定
    struct DebugCameraSettings {
        float m_mouseSensitivity = 0.3f;      ///< マウス感度（度/ピクセル）
        float m_moveSpeed = 5.0f;             ///< 移動速度（単位/秒）
        float m_fastMoveMultiplier = 3.0f;    ///< Shift押下時の移動速度倍率
        float m_pitchLimit = 89.0f;           ///< ピッチ角度制限（度）
    };

    /// @brief デバッグモード時のフリールックカメラコントローラー
    /// @note  右クリック+マウスドラッグで視点回転、WASD/QEで移動
    ///        Unityエディタライクな操作感を提供
    class DebugCameraController final : public Component {
    public:
        DebugCameraController() = default;
        ~DebugCameraController() override = default;

        /// @brief Start時に呼ばれる
        void OnStart() override;

        /// @brief 毎フレーム更新
        /// @param deltaTime フレーム経過時間
        void Update(float deltaTime) override;

        //============================================================
        // Enable/Disable
        //============================================================

        /// @brief  コントローラーを有効化
        void Enable();

        /// @brief  コントローラーを無効化
        void Disable();

        /// @brief  コントローラーが有効か確認
        /// @return 有効ならtrue
        [[nodiscard]] bool IsControllerEnabled() const;

        /// @brief  有効状態をトグル
        void ToggleEnabled();

        //============================================================
        // Settings
        //============================================================

        /// @brief  設定を取得
        /// @return カメラ設定への参照
        [[nodiscard]] DebugCameraSettings& GetSettings();

        /// @brief  設定を取得（const版）
        /// @return カメラ設定へのconst参照
        [[nodiscard]] const DebugCameraSettings& GetSettings() const;

        /// @brief  マウス感度を設定
        /// @param  sensitivity 感度値（度/ピクセル）
        void SetMouseSensitivity(float sensitivity);

        /// @brief  移動速度を設定
        /// @param  speed 速度（単位/秒）
        void SetMoveSpeed(float speed);

        //============================================================
        // State Query
        //============================================================

        /// @brief  現在カメラ操作中か確認（右クリック中）
        /// @return 操作中ならtrue
        [[nodiscard]] bool IsControlling() const;

    private:
        /// @brief  視点回転を処理
        /// @param  deltaTime フレーム経過時間
        void ProcessRotation(float deltaTime);

        /// @brief  移動を処理
        /// @param  deltaTime フレーム経過時間
        void ProcessMovement(float deltaTime);

    private:
        Transform* m_transform = nullptr;           ///< 所有者のTransform（キャッシュ）
        DebugCameraSettings m_settings;             ///< カメラ設定

        bool m_isControllerEnabled = true;          ///< コントローラー有効フラグ
        bool m_isControlling = false;               ///< 現在操作中フラグ

        float m_yaw = 0.0f;                         ///< 現在のYaw角度（度）
        float m_pitch = 0.0f;                       ///< 現在のPitch角度（度）
    };

} // namespace Engine
