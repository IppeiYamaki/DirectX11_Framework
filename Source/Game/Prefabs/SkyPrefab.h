#pragma once

#include "Engine/Math/Vector3.h"

namespace Engine { class Entity; }
namespace Engine { struct SceneContext; }

namespace Game {

    /**
     * @brief Sky�h�[���i�w�i���C���[�j
     * - Sphere �����剻���ē�����`���iCullFront�j
     * - DepthWriteOff�i���ɂ���w�i�Ƃ��Ĉ����j
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

        static Engine::Entity* Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc);
    };

} // namespace Game
