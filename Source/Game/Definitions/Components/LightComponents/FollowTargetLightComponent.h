/// @file   FollowTargetLightComponent.h
/// @brief  追従ライトコンポーネント（懐中電灯等）
#pragma once

#include "Engine/Scene/Component.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Scene/Components/LightComponent.h"

namespace Engine {
    class GameObject;
}

namespace Game {

    /// @brief 追従ライトコンポーネント
    /// @note  SpotLightを指定したGameObjectに追従させる（懐中電灯、車のヘッドライト等）
    class FollowTargetLightComponent final : public Engine::Component {
    public:
        FollowTargetLightComponent();
        ~FollowTargetLightComponent() override = default;

        void OnAwake() override;
        void Update(float deltaTime) override;

        //============================================================
        // 追従設定
        //============================================================

        /// @brief 追従対象を設定
        void SetTarget(Engine::GameObject* target) { m_target = target; }

        /// @brief 追従対象を取得
        [[nodiscard]] Engine::GameObject* GetTarget() const { return m_target; }

        /// @brief 追従オフセット（対象からの相対位置）を設定
        void SetOffset(const Engine::Vector3& offset) { m_offset = offset; }

        /// @brief 追従オフセットを取得
        [[nodiscard]] Engine::Vector3 GetOffset() const { return m_offset; }

        /// @brief 追従の滑らかさを設定（0〜1、大きいほど滑らか）
        void SetSmoothness(float smoothness) { m_smoothness = smoothness; }

        //============================================================
        // 方向設定
        //============================================================

        /// @brief 対象の向きに合わせて光の方向を設定するか
        void SetFollowTargetDirection(bool follow) { m_followTargetDirection = follow; }

        /// @brief 固定の光方向を設定（FollowTargetDirection=falseの場合に使用）
        void SetFixedDirection(const Engine::Vector3& direction) { m_fixedDirection = direction; }

        /// @brief 光の方向オフセット（ピッチ、ヨー）を設定
        void SetDirectionOffset(float pitchDegrees, float yawDegrees);

    private:
        Engine::SpotLightComponent* m_lightComponent = nullptr;
        Engine::GameObject* m_target = nullptr;

        // 追従設定
        Engine::Vector3 m_offset{ 0.0f, 1.5f, 0.5f };  // デフォルト：頭の高さ、少し前
        float m_smoothness = 0.1f;                      // 追従の滑らかさ

        // 方向設定
        bool m_followTargetDirection = true;
        Engine::Vector3 m_fixedDirection{ 0.0f, 0.0f, 1.0f };
        float m_pitchOffset = 0.0f;  // ピッチオフセット（度）
        float m_yawOffset = 0.0f;    // ヨーオフセット（度）

        // 内部状態
        Engine::Vector3 m_currentPosition{ 0.0f, 0.0f, 0.0f };
    };

} // namespace Game
