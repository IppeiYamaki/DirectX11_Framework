#pragma once

#include "Engine/Scene/Component.h"
#include "Engine/Math/Vector3.h"

namespace Game {

    /**
     * @brief 浮遊モーションコンポーネント
     * - 上下にsin波で浮遊
     * - 微細なノイズ揺れを追加
     */
    class FloatMotionComponent final : public Engine::Component {
    public:
        FloatMotionComponent();

        void Update(float deltaTime) override;

        /// @brief 浮遊の振幅を設定
        void SetAmplitude(float amplitude) { m_amplitude = amplitude; }

        /// @brief 浮遊の速度（周波数）を設定
        void SetSpeed(float speed) { m_speed = speed; }

        /// @brief ノイズの強度を設定
        void SetNoiseStrength(float noise) { m_noiseStrength = noise; }

        /// @brief 初期位置を設定
        void SetBasePosition(const Engine::Vector3& pos) { m_basePosition = pos; }

    private:
        Engine::Vector3 m_basePosition;
        float m_amplitude = 1.0f;       // 浮遊の振幅
        float m_speed = 1.0f;           // 浮遊の速度
        float m_noiseStrength = 0.1f;   // ノイズの強度
        float m_elapsedTime = 0.0f;


    };

} // namespace Game
