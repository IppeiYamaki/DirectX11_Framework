/// @file   CameraMovementQE.h
/// @brief  Q/Eキーでカメラを上下に移動させるコンポーネント
#pragma once

#include "Engine/Scene/Component.h"

namespace Game {

    /// @brief Q/Eキーでカメラを上下に移動させるコンポーネント
    class CameraMovementQE : public Engine::Component {
    public:
        /// @brief コンストラクタ
        /// @param moveSpeed 移動速度（デフォルト: 5.0）
        explicit CameraMovementQE(float moveSpeed = 5.0f);
        
        ~CameraMovementQE() override = default;

        /// @brief 毎フレーム更新
        /// @param deltaTime 前フレームからの経過時間
        void Update(float deltaTime) override;

        /// @brief 移動速度を設定
        /// @param speed 移動速度
        void SetMoveSpeed(float speed);

        /// @brief 移動速度を取得
        /// @return 現在の移動速度
        [[nodiscard]] float GetMoveSpeed() const;

    private:
        float m_moveSpeed;  ///< 移動速度
    };

} // namespace Game
