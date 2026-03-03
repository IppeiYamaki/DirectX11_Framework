/// @file   CameraFollowTarget.cpp
/// @brief  ターゲットを追従するカメラコンポーネント実装
#include "CameraFollowTarget.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"

#include <cmath>

namespace {
    constexpr float kPi = 3.14159265358979f;
    constexpr float kRadToDeg = 180.0f / kPi;
}

namespace Game {

    CameraFollowTarget::CameraFollowTarget(const Engine::Vector3& offset, float smoothSpeed)
        : m_target(nullptr)
        , m_offset(offset)
        , m_smoothSpeed(smoothSpeed)
        , m_lookAtTarget(true)
    {
    }

    void CameraFollowTarget::LateUpdate(float deltaTime) {
        if (!m_target) return;

        auto* owner = GetOwner();
        if (!owner) return;

        auto* transform = owner->GetComponent<Engine::Transform>();
        if (!transform) return;

        auto* targetTransform = m_target->GetComponent<Engine::Transform>();
        if (!targetTransform) return;

        // ターゲットの位置を取得
        Engine::Vector3 targetPos = targetTransform->GetWorldPosition();

        // 目標位置を計算（ターゲット位置 + オフセット）
        Engine::Vector3 desiredPosition = targetPos + m_offset;

        // 現在位置から目標位置へスムースに移動
        Engine::Vector3 currentPos = transform->GetWorldPosition();
        Engine::Vector3 smoothedPosition = Engine::Vector3::Lerp(
            currentPos, 
            desiredPosition, 
            m_smoothSpeed * deltaTime
        );

        transform->SetWorldPosition(smoothedPosition);

        // ターゲットを見る
        if (m_lookAtTarget) {
            // カメラからターゲットへの方向を計算
            Engine::Vector3 direction = targetPos - smoothedPosition;
            
            if (direction.LengthSquared() > Engine::Vector3::kEpsilon) {
                direction.NormalizeInPlace();
                
                // ヨーとピッチを計算
                float yaw = std::atan2(direction.x, direction.z) * kRadToDeg;
                float pitch = std::asin(-direction.y) * kRadToDeg;
                
                transform->SetYawPitchRollDegrees(yaw, pitch, 0.0f);
            }
        }
    }

    void CameraFollowTarget::SetTarget(Engine::GameObject* target) {
        m_target = target;
    }

    Engine::GameObject* CameraFollowTarget::GetTarget() const {
        return m_target;
    }

    void CameraFollowTarget::SetOffset(const Engine::Vector3& offset) {
        m_offset = offset;
    }

    const Engine::Vector3& CameraFollowTarget::GetOffset() const {
        return m_offset;
    }

    void CameraFollowTarget::SetSmoothSpeed(float speed) {
        m_smoothSpeed = speed;
    }

    float CameraFollowTarget::GetSmoothSpeed() const {
        return m_smoothSpeed;
    }

    void CameraFollowTarget::SetLookAtTarget(bool lookAt) {
        m_lookAtTarget = lookAt;
    }

    bool CameraFollowTarget::GetLookAtTarget() const {
        return m_lookAtTarget;
    }

} // namespace Game
