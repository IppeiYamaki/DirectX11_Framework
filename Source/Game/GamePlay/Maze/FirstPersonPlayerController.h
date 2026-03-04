/// @file   FirstPersonPlayerController.h
/// @brief  一人称視点プレイヤーコントローラー（WASD + マウス）
#pragma once

#include "Engine/Scene/Component.h"
#include "Engine/Math/Vector3.h"
#include "Game/Gameplay/Maze/MazeGrid.h"

namespace Engine {
    class Transform;
}

namespace Game {

    /// @brief 一人称プレイヤー設定
    struct FirstPersonPlayerSettings {
        float m_moveSpeed = 4.0f;          ///< 移動速度
        float m_mouseSensitivity = 0.2f;   ///< マウス感度
        float m_pitchLimitMin = -80.0f;    ///< ピッチ下限
        float m_pitchLimitMax = 80.0f;     ///< ピッチ上限
        float m_playerRadius = 0.3f;       ///< プレイヤー衝突半径
        float m_eyeHeight = 1.6f;          ///< 視点高さ
    };

    /// @brief 一人称視点プレイヤーコントローラー
    class FirstPersonPlayerController final : public Engine::Component {
    public:
        /// @brief コンストラクタ
        /// @param grid 迷路グリッド（衝突判定用）
        /// @param settings 設定
        /// @param cellSize セルサイズ
        explicit FirstPersonPlayerController(const MazeGrid* grid, const FirstPersonPlayerSettings& settings, float cellSize);

        ~FirstPersonPlayerController() override = default;

        //============================================================
        // ライフサイクル
        //============================================================

        void OnStart() override;
        void Update(float deltaTime) override;

        //============================================================
        // 設定
        //============================================================

        /// @brief 設定を取得
        [[nodiscard]] FirstPersonPlayerSettings& GetSettings() { return m_settings; }
        [[nodiscard]] const FirstPersonPlayerSettings& GetSettings() const { return m_settings; }

        /// @brief 移動速度を設定
        void SetMoveSpeed(float speed) { m_settings.m_moveSpeed = speed; }

        /// @brief マウス感度を設定
        void SetMouseSensitivity(float sensitivity) { m_settings.m_mouseSensitivity = sensitivity; }

        /// @brief 有効化/無効化
        void SetControlEnabled(bool enabled) { m_isControlEnabled = enabled; }
        [[nodiscard]] bool IsControlEnabled() const { return m_isControlEnabled; }

        //============================================================
        // 状態
        //============================================================

        /// @brief 現在のYaw角度を取得
        [[nodiscard]] float GetYaw() const { return m_yaw; }

        /// @brief 現在のPitch角度を取得
        [[nodiscard]] float GetPitch() const { return m_pitch; }

        /// @brief プレイヤー位置を取得（Y=0の平面座標）
        [[nodiscard]] Engine::Vector3 GetPlayerPosition() const;

    private:
        /// @brief 移動処理
        void ProcessMovement(float deltaTime);

        /// @brief 視点回転処理
        void ProcessRotation();

        /// @brief 壁との衝突を考慮した移動
        /// @param currentPos 現在位置
        /// @param desiredPos 希望移動先
        /// @return 衝突を考慮した最終位置
        Engine::Vector3 ApplyWallCollision(const Engine::Vector3& currentPos, const Engine::Vector3& desiredPos);

        /// @brief 指定位置が壁と衝突しているかチェック
        bool CheckWallCollision(float worldX, float worldZ);

    private:
        Engine::Transform* m_transform = nullptr;
        const MazeGrid* m_grid = nullptr;
        FirstPersonPlayerSettings m_settings;
        float m_cellSize = 2.0f;

        bool m_isControlEnabled = true;
        float m_yaw = 0.0f;
        float m_pitch = 0.0f;
    };

} // namespace Game
