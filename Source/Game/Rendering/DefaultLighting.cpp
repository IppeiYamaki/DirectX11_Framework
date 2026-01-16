#include "DefaultLighting.h"

#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Math/Vector4.h"

namespace Game {

    void ApplyDefaultLighting(Engine::RenderSystem& renderSystem) {
        Engine::DirectionalLight light{};
        light.m_directionX = 0.3f;
        light.m_directionY = -1.0f;
        light.m_directionZ = 0.2f;
        light.m_flags = 1;

        light.m_diffuse = Engine::Vector4(1, 1, 1, 1);
        light.m_ambient = Engine::Vector4(0.2f, 0.2f, 0.2f, 1.0f);

        renderSystem.SetLight(light);
    }

} // namespace Game
