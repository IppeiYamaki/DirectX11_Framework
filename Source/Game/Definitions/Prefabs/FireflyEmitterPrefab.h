#pragma once

#include "Engine/Math/Vector3.h"

namespace Engine { 
    struct SceneContext;
    class GameObject;
}

namespace Game {

    /**
     * @brief 蛍パーティクルエミッタープレハブ
     * - 指定された範囲内にランダムに蛍パーティクルを配置
     * - 各パーティクルはFireflyParticleComponentを持ち、独立して動く
     * - パーティクルはGameObjectとして生成され、Sceneで管理される
     */
    class FireflyEmitterPrefab final {
    public:
        struct SpawnDesc final {
            /// @brief エミッター中心位置（ワールド座標）
            Engine::Vector3 m_position{ 0, 0, 0 };

            /// @brief パーティクルが生成される範囲（各軸の半分の長さ）
            Engine::Vector3 m_spawnRange{ 40.0f, 10.0f, 40.0f };

            /// @brief 生成するパーティクル数
            int m_particleCount = 100;

            /// @brief 水面の高さ（これより下には生成しない）
            float m_waterSurfaceY = 0.0f;

            /// @brief パーティクルの最小サイズ
            float m_minSize = 0.2f;

            /// @brief パーティクルの最大サイズ
            float m_maxSize = 0.5f;

            SpawnDesc() = default;
            SpawnDesc(const Engine::Vector3& position, const Engine::Vector3& spawnRange, int particleCount = 100)
                : m_position(position), m_spawnRange(spawnRange), m_particleCount(particleCount) {
            }
        };

        static Engine::GameObject* Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc);
    };

} // namespace Game
