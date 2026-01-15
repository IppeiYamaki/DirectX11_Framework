#include "MoveTestScript.h"

#include <cstdio>

#include "Engine/Core/Logger.h"
#include "Engine/Scene/Entity.h"
#include "Engine/Scene/Components/Transform.h"

namespace Game {

    MoveTestScript::MoveTestScript(float speed)
        : m_speed(speed) {
    }

    void MoveTestScript::OnStart() {
        // OwnerからTransform取得（World::CreateEntityでTransform必須搭載済み想定）
        auto* owner = GetOwner();
        if (owner == nullptr) {
            Engine::Logger::Error("MoveTestScript: Owner is null.");
            return;
        }

        m_transform = owner->GetComponent<Engine::Transform>();
        if (m_transform == nullptr) {
            Engine::Logger::Error("MoveTestScript: Transform not found.");
            return;
        }

        Engine::Logger::Info("MoveTestScript OnStart");
    }

    void MoveTestScript::Update(float deltaTime) {
        if (m_transform == nullptr) return;

        // X方向に移動
        const Engine::Vector3 delta(m_speed * deltaTime, 0.0f, 0.0f);
        m_transform->MoveBy(delta);

        // ログ出力（毎フレームだとうるさいので1秒ごと）
        m_logTimer += deltaTime;
        if (m_logTimer >= 1.0f) {
            m_logTimer = 0.0f;

            const auto& p = m_transform->GetPosition();
            char buf[128]{};
            std::snprintf(buf, sizeof(buf), "MoveTestScript Position: (%.3f, %.3f, %.3f)", p.x, p.y, p.z);
            Engine::Logger::Info(buf);
        }
    }

} // namespace Game
