#pragma once

#include "Engine/Math/Vector3.h"

namespace Engine { class Entity; }
namespace Game { struct SceneContext; }

namespace Game {
    static Engine::Entity* Spawn(SceneContext& ctx, const SpawnDesc& desc);

    /**
     * @brief UnityのPrefab相当（コード版）
     * - MeshType / Material / Transform 初期値 / Script などをここで完結させる
     */
    class SamplePrefab final {
    public:
        struct SpawnDesc final {
            Engine::Vector3 m_position{ 0,0,0 };
            float m_uniformScale = 1.0f;
            float m_rotateDegPerSec = 45.0f;

            SpawnDesc() = default;

            // ctx.Spawn<Prefab>(pos, scale, rot) 用
            SpawnDesc(const Engine::Vector3& position,
                float uniformScale = 1.0f,
                float rotateDegPerSec = 45.0f)
                : m_position(position)
                , m_uniformScale(uniformScale)
                , m_rotateDegPerSec(rotateDegPerSec) {
            }
        };

        static Engine::Entity* Spawn(SceneContext& ctx, const SpawnDesc& desc);
    };

} // namespace Game
