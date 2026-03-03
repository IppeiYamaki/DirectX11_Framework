#include "FloatMotionComponent.h"

#include <cmath>

#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"

namespace Game {

    FloatMotionComponent::FloatMotionComponent()
        : m_basePosition(Engine::Vector3::Zero())
        , m_amplitude(1.0f)
        , m_speed(1.0f)
        , m_noiseStrength(0.1f)
        , m_elapsedTime(0.0f) {
    }

    void FloatMotionComponent::Update(float deltaTime) {
        m_elapsedTime += deltaTime;

        auto* owner = GetOwner();
        if (!owner) return;

        auto* transform = owner->GetComponent<Engine::Transform>();
        if (!transform) return;

        // 基準位置が未設定の場合は現在位置を基準とする
        if (m_basePosition.x == 0.0f && m_basePosition.y == 0.0f && m_basePosition.z == 0.0f) {
            m_basePosition = transform->GetPosition();
        }

        // Sin波による上下浮遊
        float verticalOffset = std::sin(m_elapsedTime * m_speed) * m_amplitude;

        // 簡易ノイズ（複数のsin波を合成）
        float noiseX = std::sin(m_elapsedTime * 2.3f) * m_noiseStrength * 0.5f +
                       std::sin(m_elapsedTime * 1.7f) * m_noiseStrength * 0.3f;
        float noiseY = std::sin(m_elapsedTime * 1.9f) * m_noiseStrength * 0.4f;
        float noiseZ = std::sin(m_elapsedTime * 2.1f) * m_noiseStrength * 0.3f +
                       std::cos(m_elapsedTime * 1.5f) * m_noiseStrength * 0.2f;

        // 新しい位置を計算
        Engine::Vector3 newPosition;
        newPosition.x = m_basePosition.x + noiseX;
        newPosition.y = m_basePosition.y + verticalOffset + noiseY;
        newPosition.z = m_basePosition.z + noiseZ;

        transform->SetPosition(newPosition);
    }

} // namespace Game
