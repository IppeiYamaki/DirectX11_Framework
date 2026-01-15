#pragma once

#include "Engine/Scene/Components/ScriptComponent.h"
#include "Engine/Math/Vector3.h"

namespace Engine { class Transform; }

namespace Game {

    /**
     * @brief Transformを毎フレーム MoveBy するテスト
     * - 1秒ごとに位置をログ出力
     */
    class MoveTestScript final : public Engine::ScriptComponent {
    public:
        explicit MoveTestScript(float speed = 1.0f);

        void OnStart() override;
        void Update(float deltaTime) override;

    private:
        Engine::Transform* m_transform = nullptr; // 借用
        float m_speed       = 1.0f;

        float m_logTimer    = 0.0f;
    };

} // namespace Game
