#pragma once

#include "Engine/Scene/Component.h"
#include "Engine/Math/Vector3.h"

namespace Game {

    /**
     * @brief サンプル：回転し続けるコンポーネント
     */
    class SkyFollowCameraComponent final : public Engine::Component {
    public:

        void Update(float deltaTime) override;

    };

} // namespace Game
