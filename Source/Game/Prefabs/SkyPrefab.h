#pragma once

#include "Engine/Math/Vector3.h"
#include "Engine/Scene/Prefab.h"

namespace Engine { class Entity; }
namespace Engine { struct SceneContext; }

namespace Game {

    struct SkySpawnDesc final {
        Engine::Vector3 m_position{ 0, 0, 0 };
        float m_uniformScale = 200.0f;

        SkySpawnDesc() = default;
        SkySpawnDesc(const Engine::Vector3& position, float uniformScale = 200.0f)
            : m_position(position), m_uniformScale(uniformScale) {
        }
    };

    /**
     * @brief Sky�h�[���i�w�i���C���[�j
     * - Sphere �����剻���ē�����`���iCullFront�j
     * - DepthWriteOff�i���ɂ���w�i�Ƃ��Ĉ����j
     */
    class SkyPrefab final : public Engine::Prefab<SkySpawnDesc> {
    public:
        using SpawnDesc = SkySpawnDesc;

        Engine::Entity* Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc) override;
    };

} // namespace Game
