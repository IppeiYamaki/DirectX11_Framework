#pragma once

#include "Engine/Math/Vector3.h"
#include "Engine/Scene/Prefab.h"

namespace Engine { class Entity; }
namespace Engine { struct SceneContext; }

namespace Game {

    struct SampleSpawnDesc final {
        Engine::Vector3 m_position{ 0,0,0 };
        float m_uniformScale = 1.0f;
        float m_rotateDegPerSec = 45.0f;

        SampleSpawnDesc() = default;

        // ctx.SpawnPrefab<SamplePrefab>(pos, scale, rot) ���\�ɂ���
        SampleSpawnDesc(const Engine::Vector3& position,
            float uniformScale = 1.0f,
            float rotateDegPerSec = 45.0f)
            : m_position(position)
            , m_uniformScale(uniformScale)
            , m_rotateDegPerSec(rotateDegPerSec) {
        }
    };

    /**
     * @brief Unity��Prefab�����i�R�[�h�Łj
     * - MeshType / Material / Transform �����l / Script �Ȃǂ������Ŋ���������
     */
    class SamplePrefab final : public Engine::Prefab<SampleSpawnDesc> {
    public:
        using SpawnDesc = SampleSpawnDesc;

        Engine::Entity* Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc) override;
    };

} // namespace Game
