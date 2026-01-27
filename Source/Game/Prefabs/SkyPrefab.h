#pragma once

#include "Engine/Math/Vector3.h"
#include "Engine/Scene/SceneContext.h"

namespace Engine { class GameObject; }

namespace Game {

    /**
     * @brief Skyドーム（背景レイヤー）
     * - Sphere を大きく化して内側を描画（CullFront）
     * - DepthWriteOff（常に一番背景として扱う）
     */
    class SkyPrefab final {
    public:
        struct SpawnDesc final {
            Engine::Vector3 m_position{ 0, 0, 0 };
            float m_uniformScale = 200.0f;

            SpawnDesc() = default;
            SpawnDesc(const Engine::Vector3& position, float uniformScale = 200.0f)
                : m_position(position), m_uniformScale(uniformScale) {
            }
        };

        static Engine::GameObject* Spawn(Game::SceneContext& ctx, const SpawnDesc& desc);
    };

} // namespace Game
