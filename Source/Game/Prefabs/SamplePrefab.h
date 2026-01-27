#pragma once

#include "Engine/Math/Vector3.h"

namespace Engine { class GameObject; }
namespace Engine { struct SceneContext; }

namespace Game {

    /**
     * @brief Unity風Prefabパターン（コードで）
     * - MeshType / Material / Transform 初期値 / Script などを一箇所で完結させる
     */
    class SamplePrefab final {
    public:
        struct SpawnDesc final {
            Engine::Vector3 m_position{ 0,0,0 };
            float m_uniformScale = 1.0f;
            float m_rotateDegPerSec = 45.0f;

            SpawnDesc() = default;

            // ctx.Spawn<SamplePrefab>(pos, scale, rot) を可能にする
            SpawnDesc(const Engine::Vector3& position,
                float uniformScale = 1.0f,
                float rotateDegPerSec = 45.0f)
                : m_position(position)
                , m_uniformScale(uniformScale)
                , m_rotateDegPerSec(rotateDegPerSec) {
            }
        };

        static Engine::GameObject* Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc);
    };

} // namespace Game
