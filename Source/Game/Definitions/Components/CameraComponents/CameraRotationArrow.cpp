/// @file   CameraRotationArrow.cpp
/// @brief  矢印キーでカメラを回転させるコンポーネント実装
#include "CameraRotationArrow.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Platform/Input.h"

#include <algorithm>

namespace Game {

    CameraRotationArrow::CameraRotationArrow(float rotationSpeed)
        : m_rotationSpeed(rotationSpeed)
        , m_minPitch(-89.0f)
        , m_maxPitch(89.0f)
        , m_currentYaw(0.0f)
        , m_currentPitch(0.0f)
    {
    }

    void CameraRotationArrow::OnStart() {
        auto* owner = GetOwner();
        if (!owner) return;

        auto* transform = owner->GetComponent<Engine::Transform>();
        if (!transform) return;

        // 現在の回転から初期値を取得（x = pitch, y = yaw）
        const Engine::Vector3& rotation = transform->GetRotationEulerDegrees();
        m_currentYaw = rotation.Y();
        m_currentPitch = rotation.X();
    }

    void CameraRotationArrow::Update(float deltaTime) {
        auto* owner = GetOwner();
        if (!owner) return;

        auto* transform = owner->GetComponent<Engine::Transform>();
        if (!transform) return;

        float yawDelta = 0.0f;
        float pitchDelta = 0.0f;

        // 矢印キー入力を取得
        if (Engine::Input::IsKeyPressed(VK_LEFT)) {
            yawDelta -= m_rotationSpeed * deltaTime;  // 左回転
        }
        if (Engine::Input::IsKeyPressed(VK_RIGHT)) {
            yawDelta += m_rotationSpeed * deltaTime;  // 右回転
        }
        if (Engine::Input::IsKeyPressed(VK_UP)) {
            pitchDelta -= m_rotationSpeed * deltaTime;  // 下を向く
        }
        if (Engine::Input::IsKeyPressed(VK_DOWN)) {
            pitchDelta += m_rotationSpeed * deltaTime;  // 上を向く
        }

        // ヨーとピッチを更新
        if (yawDelta != 0.0f || pitchDelta != 0.0f) {
            m_currentYaw += yawDelta;
            m_currentPitch += pitchDelta;

            // ピッチを制限
            m_currentPitch = std::clamp(m_currentPitch, m_minPitch, m_maxPitch);

            // 回転を適用
            transform->SetYawPitchRollDegrees(m_currentYaw, m_currentPitch, 0.0f);
        }
    }

    void CameraRotationArrow::SetRotationSpeed(float speed) {
        m_rotationSpeed = speed;
    }

    float CameraRotationArrow::GetRotationSpeed() const {
        return m_rotationSpeed;
    }

    void CameraRotationArrow::SetPitchLimits(float minPitch, float maxPitch) {
        m_minPitch = minPitch;
        m_maxPitch = maxPitch;
    }

} // namespace Game
