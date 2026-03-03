/// @file   CameraFollowTarget.h
/// @brief  ターゲットを追従するカメラコンポーネント
#pragma once

#include "Engine/Scene/Component.h"
#include "Engine/Math/Vector3.h"

namespace Engine {
    class GameObject;
}

namespace Game {

    /// @brief ターゲットを追従するカメラコンポーネント
    class CameraFollowTarget : public Engine::Component {
    public:
        /// @brief コンストラクタ
        /// @param offset ターゲットからのオフセット（デフォルト: (0, 5, -10)）
        /// @param smoothSpeed 追従のスムース速度（デフォルト: 5.0）
        explicit CameraFollowTarget(
            const Engine::Vector3& offset = Engine::Vector3(0.0f, 5.0f, -10.0f),
            float smoothSpeed = 5.0f
        );
        
        ~CameraFollowTarget() override = default;

        /// @brief 毎フレーム遅延更新
        /// @param deltaTime 前フレームからの経過時間
        void LateUpdate(float deltaTime) override;

        /// @brief 追従対象を設定
        /// @param target 追従対象のGameObject
        void SetTarget(Engine::GameObject* target);

        /// @brief 追従対象を取得
        /// @return 追従対象のGameObject
        [[nodiscard]] Engine::GameObject* GetTarget() const;

        /// @brief オフセットを設定
        /// @param offset ターゲットからのオフセット
        void SetOffset(const Engine::Vector3& offset);

        /// @brief オフセットを取得
        /// @return 現在のオフセット
        [[nodiscard]] const Engine::Vector3& GetOffset() const;

        /// @brief スムース速度を設定
        /// @param speed スムース速度
        void SetSmoothSpeed(float speed);

        /// @brief スムース速度を取得
        /// @return 現在のスムース速度
        [[nodiscard]] float GetSmoothSpeed() const;

        /// @brief カメラがターゲットを見るかどうかを設定
        /// @param lookAt trueならターゲットを見る
        void SetLookAtTarget(bool lookAt);

        /// @brief カメラがターゲットを見るかどうかを取得
        /// @return trueならターゲットを見る
        [[nodiscard]] bool GetLookAtTarget() const;

    private:
        Engine::GameObject* m_target;  ///< 追従対象
        Engine::Vector3 m_offset;      ///< オフセット
        float m_smoothSpeed;           ///< スムース速度
        bool m_lookAtTarget;           ///< ターゲットを見るかどうか
    };

} // namespace Game
