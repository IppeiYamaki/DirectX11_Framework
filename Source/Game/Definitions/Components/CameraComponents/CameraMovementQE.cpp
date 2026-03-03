/// @file   CameraMovementQE.cpp
/// @brief  Q/Eキーでカメラを上下に移動させるコンポーネント実装
#include "CameraMovementQE.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Platform/Input.h"

namespace Game {

    CameraMovementQE::CameraMovementQE(float moveSpeed)
        : m_moveSpeed(moveSpeed)
    {
    }

    void CameraMovementQE::Update(float deltaTime) {
        auto* owner = GetOwner();
        if (!owner) return;

        auto* transform = owner->GetComponent<Engine::Transform>();
        if (!transform) return;

        Engine::Vector3 moveDir = Engine::Vector3::Zero();

        // Q/Eキー入力を取得（上下移動）
        if (Engine::Input::IsKeyPressed('Q')) {
            moveDir -= Engine::Vector3::UnitY();  // 下降
        }
        if (Engine::Input::IsKeyPressed('E')) {
            moveDir += Engine::Vector3::UnitY();  // 上昇
        }

        // 移動量を適用（上下のみなので正規化は不要）
        if (moveDir.LengthSquared() > 0.0f) {
            transform->AddPosition(moveDir * m_moveSpeed * deltaTime);
        }
    }

    void CameraMovementQE::SetMoveSpeed(float speed) {
        m_moveSpeed = speed;
    }

    float CameraMovementQE::GetMoveSpeed() const {
        return m_moveSpeed;
    }

} // namespace Game
