/// @file   FollowTargetLightComponent.cpp
/// @brief  追従ライトコンポーネント実装
#include "FollowTargetLightComponent.h"

#include <cmath>

#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/LightComponent.h"

namespace {
    constexpr float kPi = 3.14159265358979323846f;
    constexpr float kDegToRad = kPi / 180.0f;

    /// @brief 2つのVector3を線形補間
    Engine::Vector3 LerpVector3(const Engine::Vector3& a, const Engine::Vector3& b, float t) {
        return Engine::Vector3(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t
        );
    }
}

namespace Game {

    FollowTargetLightComponent::FollowTargetLightComponent()
        : m_target(nullptr)
        , m_offset(0.0f, 1.5f, 0.5f)
        , m_smoothness(0.1f)
        , m_followTargetDirection(true)
        , m_fixedDirection(0.0f, 0.0f, 1.0f)
        , m_pitchOffset(0.0f)
        , m_yawOffset(0.0f)
        , m_currentPosition(0.0f, 0.0f, 0.0f) {
    }

    void FollowTargetLightComponent::OnAwake() {
        auto* owner = GetOwner();
        if (owner) {
            m_lightComponent = owner->GetComponent<Engine::SpotLightComponent>();

            // 初期位置を設定
            if (auto* transform = owner->GetComponent<Engine::Transform>()) {
                m_currentPosition = transform->GetPosition();
            }
        }
    }

    void FollowTargetLightComponent::Update(float deltaTime) {
        if (!m_lightComponent) return;

        auto* owner = GetOwner();
        if (!owner) return;

        auto* ownerTransform = owner->GetComponent<Engine::Transform>();
        if (!ownerTransform) return;

        // 追従対象がある場合
        if (m_target) {
            auto* targetTransform = m_target->GetComponent<Engine::Transform>();
            if (targetTransform) {
                // 目標位置を計算（対象位置 + オフセット）
                Engine::Vector3 targetPos = targetTransform->GetWorldPosition();

                // オフセットを対象の回転に合わせて変換
                Engine::Vector3 targetForward = targetTransform->GetForward();
                Engine::Vector3 targetRight = targetTransform->GetRight();
                Engine::Vector3 targetUp = Engine::Vector3(0.0f, 1.0f, 0.0f);  // ワールド上方向

                Engine::Vector3 worldOffset;
                worldOffset.x = targetRight.x * m_offset.x + targetUp.x * m_offset.y + targetForward.x * m_offset.z;
                worldOffset.y = targetRight.y * m_offset.x + targetUp.y * m_offset.y + targetForward.y * m_offset.z;
                worldOffset.z = targetRight.z * m_offset.x + targetUp.z * m_offset.y + targetForward.z * m_offset.z;

                Engine::Vector3 goalPosition;
                goalPosition.x = targetPos.x + worldOffset.x;
                goalPosition.y = targetPos.y + worldOffset.y;
                goalPosition.z = targetPos.z + worldOffset.z;

                // 滑らかに追従
                float lerpFactor = 1.0f - m_smoothness;
                lerpFactor = std::min(1.0f, std::max(0.0f, lerpFactor * deltaTime * 60.0f));  // 60FPS基準で正規化
                m_currentPosition = LerpVector3(m_currentPosition, goalPosition, lerpFactor);

                // 位置を更新
                ownerTransform->SetPosition(m_currentPosition);

                // 方向を更新
                if (m_followTargetDirection) {
                    Engine::Vector3 direction = targetForward;

                    // ピッチ・ヨーオフセットを適用
                    if (m_pitchOffset != 0.0f || m_yawOffset != 0.0f) {
                        float pitchRad = m_pitchOffset * kDegToRad;
                        float yawRad = m_yawOffset * kDegToRad;

                        float cosPitch = std::cos(pitchRad);
                        float sinPitch = std::sin(pitchRad);
                        float cosYaw = std::cos(yawRad);
                        float sinYaw = std::sin(yawRad);

                        // ヨー回転（Y軸周り）を適用
                        Engine::Vector3 yawRotated;
                        yawRotated.x = direction.x * cosYaw - direction.z * sinYaw;
                        yawRotated.y = direction.y;
                        yawRotated.z = direction.x * sinYaw + direction.z * cosYaw;

                        // ピッチ回転（X軸周り）を適用
                        Engine::Vector3 rotated;
                        rotated.x = yawRotated.x;
                        rotated.y = yawRotated.y * cosPitch - yawRotated.z * sinPitch;
                        rotated.z = yawRotated.y * sinPitch + yawRotated.z * cosPitch;

                        direction = rotated.Normalized();
                    }

                    m_lightComponent->SetDirection(direction);
                }
                else {
                    m_lightComponent->SetDirection(m_fixedDirection);
                }
            }
        }
    }

    void FollowTargetLightComponent::SetDirectionOffset(float pitchDegrees, float yawDegrees) {
        m_pitchOffset = pitchDegrees;
        m_yawOffset = yawDegrees;
    }

} // namespace Game
