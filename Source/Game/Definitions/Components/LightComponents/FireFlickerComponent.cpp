/// @file   FireFlickerComponent.cpp
/// @brief  炎の揺らぎエフェクト用コンポーネント実装
#include "FireFlickerComponent.h"

#include <cmath>
#include <cstdlib>

#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/LightComponent.h"

namespace Game {

    FireFlickerComponent::FireFlickerComponent()
        : m_basePosition(Engine::Vector3::Zero())
        , m_baseColor(1.0f, 0.6f, 0.2f)
        , m_baseIntensity(2.0f)
        , m_intensityFlickerRange(0.3f)
        , m_positionFlickerRange(0.1f)
        , m_colorFlickerRange(0.1f)
        , m_flickerSpeed(8.0f)
        , m_elapsedTime(0.0f)
        , m_randomSeed(0.0f)
        , m_basePositionSet(false) {
    }

    void FireFlickerComponent::OnAwake() {
        // PointLightComponentを取得
        auto* owner = GetOwner();
        if (owner) {
            m_lightComponent = owner->GetComponent<Engine::PointLightComponent>();

            // 基準位置が未設定の場合のみTransformから取得
            if (!m_basePositionSet) {
                if (auto* transform = owner->GetComponent<Engine::Transform>()) {
                    m_basePosition = transform->GetPosition();
                }
            }

            // 基準色を取得
            if (m_lightComponent) {
                m_baseColor = m_lightComponent->GetColor();
                m_baseIntensity = m_lightComponent->GetIntensity();
            }
        }

        // ランダムシードを初期化（各インスタンスで異なる揺らぎパターン）
        m_randomSeed = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 100.0f;
    }

    void FireFlickerComponent::Update(float deltaTime) {
        if (!m_lightComponent) return;

        m_elapsedTime += deltaTime;
        float t = m_elapsedTime * m_flickerSpeed + m_randomSeed;

        // 複数のsin波を合成して自然な揺らぎを生成
        float noise1 = std::sin(t * 1.0f) * 0.5f;
        float noise2 = std::sin(t * 2.3f) * 0.3f;
        float noise3 = std::sin(t * 3.7f) * 0.2f;
        float combinedNoise = noise1 + noise2 + noise3;

        // 強度の揺らぎ
        float intensityOffset = combinedNoise * m_intensityFlickerRange * m_baseIntensity;
        float newIntensity = m_baseIntensity + intensityOffset;
        newIntensity = std::max(0.1f, newIntensity);  // 最低輝度を確保
        m_lightComponent->SetIntensity(newIntensity);

        // 色の揺らぎ（暖色の範囲で変動）
        float colorOffset = combinedNoise * m_colorFlickerRange;
        Engine::Vector3 newColor;
        newColor.x = std::min(1.0f, std::max(0.0f, m_baseColor.x + colorOffset * 0.5f));
        newColor.y = std::min(1.0f, std::max(0.0f, m_baseColor.y + colorOffset));
        newColor.z = std::min(1.0f, std::max(0.0f, m_baseColor.z + colorOffset * 0.3f));
        m_lightComponent->SetColor(newColor);

        // 位置の揺らぎ
        auto* owner = GetOwner();
        if (owner) {
            if (auto* transform = owner->GetComponent<Engine::Transform>()) {
                float posNoiseX = std::sin(t * 1.5f + 1.0f) * m_positionFlickerRange;
                float posNoiseY = std::sin(t * 1.8f + 2.0f) * m_positionFlickerRange * 0.5f;
                float posNoiseZ = std::sin(t * 1.3f + 3.0f) * m_positionFlickerRange;

                Engine::Vector3 newPosition;
                newPosition.x = m_basePosition.x + posNoiseX;
                newPosition.y = m_basePosition.y + posNoiseY;
                newPosition.z = m_basePosition.z + posNoiseZ;
                transform->SetPosition(newPosition);
            }
        }
    }

} // namespace Game
