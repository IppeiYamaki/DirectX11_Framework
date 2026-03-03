#include "FireflyParticleComponent.h"

#include <cmath>
#include <random>

#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"

namespace Game {

    FireflyParticleComponent::FireflyParticleComponent()
        : m_basePosition(Engine::Vector3::Zero())
        , m_driftRange(Engine::Vector3(3.0f, 2.0f, 3.0f))
        , m_driftSpeed(1.0f)
        , m_flickerSpeed(2.0f)
        , m_minBrightness(0.2f)
        , m_baseColor(Engine::Color(1.0f, 1.0f, 0.8f, 1.0f))
        , m_size(0.3f)
        , m_elapsedTime(0.0f)
        , m_phaseOffset(0.0f)
        , m_noiseOffsetX(0.0f)
        , m_noiseOffsetY(0.0f)
        , m_noiseOffsetZ(0.0f)
        , m_flickerPhaseOffset(0.0f)
        , m_currentBrightness(1.0f) {
    }

    void FireflyParticleComponent::OnStart() {
        // ランダムな位相オフセットを生成（各パーティクルが独立して動く）
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(0.0f, 6.28318f); // 0～2π

        m_phaseOffset = dis(gen);
        m_noiseOffsetX = dis(gen);
        m_noiseOffsetY = dis(gen);
        m_noiseOffsetZ = dis(gen);
        m_flickerPhaseOffset = dis(gen);

        // 初期位置を設定
        auto* owner = GetOwner();
        if (owner) {
            if (auto* transform = owner->GetComponent<Engine::Transform>()) {
                if (m_basePosition.x == 0.0f && m_basePosition.y == 0.0f && m_basePosition.z == 0.0f) {
                    m_basePosition = transform->GetPosition();
                }
            }
        }
    }

    void FireflyParticleComponent::Update(float deltaTime) {
        m_elapsedTime += deltaTime;

        auto* owner = GetOwner();
        if (!owner) return;

        auto* transform = owner->GetComponent<Engine::Transform>();
        if (!transform) return;

        // 漂い動き（複数のsin波を組み合わせて自然な動きを作る）
        float time = m_elapsedTime * m_driftSpeed;

        // X方向の漂い（2つのsin波を合成）
        float driftX = std::sin(time + m_phaseOffset + m_noiseOffsetX) * m_driftRange.x * 0.5f +
                       std::sin(time * 0.7f + m_noiseOffsetX) * m_driftRange.x * 0.3f;

        // Y方向の漂い（上下の動き）
        float driftY = std::sin(time * 0.8f + m_phaseOffset + m_noiseOffsetY) * m_driftRange.y * 0.6f +
                       std::cos(time * 1.3f + m_noiseOffsetY) * m_driftRange.y * 0.4f;

        // Z方向の漂い
        float driftZ = std::cos(time * 0.9f + m_phaseOffset + m_noiseOffsetZ) * m_driftRange.z * 0.5f +
                       std::sin(time * 0.6f + m_noiseOffsetZ) * m_driftRange.z * 0.3f;

        // 新しい位置を計算
        Engine::Vector3 newPosition;
        newPosition.x = m_basePosition.x + driftX;
        newPosition.y = m_basePosition.y + driftY;
        newPosition.z = m_basePosition.z + driftZ;

        transform->SetPosition(newPosition);

        // 明滅（sinとnoiseを組み合わせ）
        float flickerTime = m_elapsedTime * m_flickerSpeed;
        float flicker1 = std::sin(flickerTime + m_flickerPhaseOffset) * 0.5f + 0.5f;
        float flicker2 = std::sin(flickerTime * 2.3f + m_flickerPhaseOffset * 0.7f) * 0.5f + 0.5f;
        
        // 輝度を計算（最小輝度と最大輝度の間で変化）
        m_currentBrightness = m_minBrightness + (1.0f - m_minBrightness) * (flicker1 * 0.7f + flicker2 * 0.3f);
    }

    Engine::Color FireflyParticleComponent::GetCurrentColor() const {
        // 輝度を色に反映
        return Engine::Color(
            m_baseColor.x * m_currentBrightness,
            m_baseColor.y * m_currentBrightness,
            m_baseColor.z * m_currentBrightness,
            m_baseColor.w
        );
    }

} // namespace Game
