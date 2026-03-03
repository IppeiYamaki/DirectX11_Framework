/// @file   CameraRotationArrow.h
/// @brief  矢印キーでカメラを回転させるコンポーネント
#pragma once

#include "Engine/Scene/Component.h"

namespace Game {

    /// @brief 矢印キーでカメラを回転させるコンポーネント
    class CameraRotationArrow : public Engine::Component {
    public:
        /// @brief コンストラクタ
        /// @param rotationSpeed 回転速度（度/秒、デフォルト: 90.0）
        explicit CameraRotationArrow(float rotationSpeed = 90.0f);
        
        ~CameraRotationArrow() override = default;

        /// @brief Start時に呼ばれる（初期回転を取得）
        void OnStart() override;

        /// @brief 毎フレーム更新
        /// @param deltaTime 前フレームからの経過時間
        void Update(float deltaTime) override;

        /// @brief 回転速度を設定
        /// @param speed 回転速度（度/秒）
        void SetRotationSpeed(float speed);

        /// @brief 回転速度を取得
        /// @return 現在の回転速度（度/秒）
        [[nodiscard]] float GetRotationSpeed() const;

        /// @brief ピッチ制限を設定
        /// @param minPitch 最小ピッチ角度（度）
        /// @param maxPitch 最大ピッチ角度（度）
        void SetPitchLimits(float minPitch, float maxPitch);

    private:
        float m_rotationSpeed;  ///< 回転速度（度/秒）
        float m_minPitch;       ///< 最小ピッチ角度
        float m_maxPitch;       ///< 最大ピッチ角度
        float m_currentYaw;     ///< 現在のヨー角
        float m_currentPitch;   ///< 現在のピッチ角
    };

} // namespace Game
