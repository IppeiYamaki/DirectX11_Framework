/// @file   SunCycleComponent.cpp
/// @brief  太陽光の日周変化コンポーネント実装
#include "SunCycleComponent.h"

#include <cmath>

#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/LightComponent.h"

namespace {
    constexpr float kPi = 3.14159265358979323846f;

    /// @brief 2つのVector3を線形補間
    Engine::Vector3 LerpVector3(const Engine::Vector3& a, const Engine::Vector3& b, float t) {
        return Engine::Vector3(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t
        );
    }

    /// @brief スムーズステップ補間
    float SmoothStep(float t) {
        return t * t * (3.0f - 2.0f * t);
    }
}

namespace Game {

    SunCycleComponent::SunCycleComponent()
        : m_dayCycleDuration(120.0f)
        , m_timeOfDay(0.5f)
        , m_isPaused(false)
        , m_dawnColor(1.0f, 0.6f, 0.4f)
        , m_noonColor(1.0f, 1.0f, 0.95f)
        , m_duskColor(1.0f, 0.5f, 0.3f)
        , m_nightColor(0.3f, 0.4f, 0.6f)
        , m_noonIntensity(1.5f)
        , m_nightIntensity(0.2f) {
    }

    void SunCycleComponent::OnAwake() {
        auto* owner = GetOwner();
        if (owner) {
            m_lightComponent = owner->GetComponent<Engine::DirectionalLightComponent>();
        }
    }

    void SunCycleComponent::Update(float deltaTime) {
        if (!m_lightComponent || m_isPaused) return;

        // 時刻を進める
        if (m_dayCycleDuration > 0.0f) {
            m_timeOfDay += deltaTime / m_dayCycleDuration;
            if (m_timeOfDay >= 1.0f) {
                m_timeOfDay -= 1.0f;
            }
        }

        // 方向を更新
        Engine::Vector3 direction = CalculateSunDirection();
        m_lightComponent->SetDirection(direction);

        // 色を更新
        Engine::Vector3 color = CalculateSunColor();
        m_lightComponent->SetColor(color);

        // 強度を更新
        float intensity = CalculateSunIntensity();
        m_lightComponent->SetIntensity(intensity);
    }

    Engine::Vector3 SunCycleComponent::CalculateSunDirection() const {
        // 時刻を角度に変換（0=真夜中=真下、0.5=正午=真上斜め前）
        float angle = m_timeOfDay * 2.0f * kPi;

        // Y軸（上下）：-cos(angle)で、真夜中(angle=0)で-1（地平線下）、正午(angle=π)で+1（真上から）
        // 太陽は東から昇って西に沈む
        float y = -std::cos(angle);  // 真夜中で-1（下向き）、正午で+1（上向きから照らす）
        float xz = std::sin(angle);  // 東西方向の移動

        // 水平角度を加味（東=正のX、西=負のX）
        float x = xz * 0.5f;
        float z = xz * 0.866f;  // sin(60度)で斜め前方向

        return Engine::Vector3(x, y, z).Normalized();
    }

    Engine::Vector3 SunCycleComponent::CalculateSunColor() const {
        // 時刻に応じた色を計算
        // 0.0 = 真夜中
        // 0.25 = 朝（日の出）
        // 0.5 = 正午
        // 0.75 = 夕方（日没）
        // 1.0 = 真夜中

        if (m_timeOfDay < 0.2f) {
            // 真夜中〜夜明け前
            float t = SmoothStep(m_timeOfDay / 0.2f);
            return LerpVector3(m_nightColor, m_dawnColor, t);
        }
        else if (m_timeOfDay < 0.3f) {
            // 朝焼け〜午前
            float t = SmoothStep((m_timeOfDay - 0.2f) / 0.1f);
            return LerpVector3(m_dawnColor, m_noonColor, t);
        }
        else if (m_timeOfDay < 0.7f) {
            // 日中
            return m_noonColor;
        }
        else if (m_timeOfDay < 0.8f) {
            // 午後〜夕焼け
            float t = SmoothStep((m_timeOfDay - 0.7f) / 0.1f);
            return LerpVector3(m_noonColor, m_duskColor, t);
        }
        else if (m_timeOfDay < 0.9f) {
            // 夕焼け〜夜
            float t = SmoothStep((m_timeOfDay - 0.8f) / 0.1f);
            return LerpVector3(m_duskColor, m_nightColor, t);
        }
        else {
            // 夜
            return m_nightColor;
        }
    }

    float SunCycleComponent::CalculateSunIntensity() const {
        // 日中は明るく、夜は暗い
        // sin波で滑らかに変化
        float angle = m_timeOfDay * 2.0f * kPi;
        float sinValue = std::sin(angle - kPi * 0.5f);  // 正午でmax、真夜中でmin

        // 0〜1の範囲に正規化
        float normalized = (sinValue + 1.0f) * 0.5f;

        // 強度を補間
        return m_nightIntensity + (m_noonIntensity - m_nightIntensity) * normalized;
    }

} // namespace Game
