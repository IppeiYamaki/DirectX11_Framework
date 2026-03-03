/// @file   CameraMovementWASD.cpp
/// @brief  WASDキーでカメラを移動させるコンポーネント実装
#include "CameraMovementWASD.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Platform/Input.h"

namespace Game {

    CameraMovementWASD::CameraMovementWASD(float moveSpeed)
        : m_moveSpeed(moveSpeed)
    {
    }

    void CameraMovementWASD::Update(float deltaTime) {
        auto* owner = GetOwner();
        if (!owner) return;

        auto* transform = owner->GetComponent<Engine::Transform>();
        if (!transform) return;

        Engine::Vector3 moveDir = Engine::Vector3::Zero();

        // WASDキー入力を取得
        if (Engine::Input::IsKeyPressed('W')) {
            moveDir += transform->GetForward();
        }
        if (Engine::Input::IsKeyPressed('S')) {
            moveDir -= transform->GetForward();
        }
        if (Engine::Input::IsKeyPressed('A')) {
            moveDir -= transform->GetRight();
        }
        if (Engine::Input::IsKeyPressed('D')) {
            moveDir += transform->GetRight();
        }

        // 移動量を正規化して速度を適用
        if (moveDir.LengthSquared() > 0.0f) {
            moveDir.NormalizeInPlace();
            transform->AddPosition(moveDir * m_moveSpeed * deltaTime);
        }
    }

    void CameraMovementWASD::SetMoveSpeed(float speed) {
        m_moveSpeed = speed;
    }

    float CameraMovementWASD::GetMoveSpeed() const {
        return m_moveSpeed;
    }

} // namespace Game
