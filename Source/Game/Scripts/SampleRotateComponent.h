#pragma once

#include "Engine/Scene/Component.h"
#include "Engine/Math/Vector3.h"

namespace Game {

    /**
     * @brief サンプル：回転し続けるコンポーネント
     */
    class SampleRotateComponent final : public Engine::Component {
    public:
        explicit SampleRotateComponent(float degreesPerSecond = 45.0f);

        void Update(float deltaTime) override;

    private:
        float m_degreesPerSecond = 45.0f;
    };

} // namespace Game
