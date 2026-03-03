#pragma once

#include "Engine/Math/Vector3.h"

namespace Engine { 
    struct SceneContext;
    class GameObject;
    class PlanarReflectionSystem;
}

namespace Game {

    /**
     * @brief 水面プレハブ（XZ平面上のMeshField）
     * - XZ平面上に配置された格子状メッシュ（MeshField）を生成
     * - 法線は上向き(0,1,0)、Y軸が高さ方向
     * - 平面反射と波アニメーション付き
     * - 頂点単位で波の変位が可能（将来の拡張性あり）
     */
    class WaterPlanePrefab final {
    public:
        struct SpawnDesc final {
            /// @brief 水面の位置（ワールド座標）
            Engine::Vector3 m_position{ 0, 0, 0 };

            /// @brief 水面の幅（X方向）
            float m_width = 100.0f;

            /// @brief 水面の奥行き（Z方向）
            float m_depth = 100.0f;

            /// @brief メッシュのX方向分割数
            int m_gridX = 30;

            /// @brief メッシュのZ方向分割数
            int m_gridZ = 30;

            /// @brief 平面反射システムへの参照
            Engine::PlanarReflectionSystem* m_reflectionSystem = nullptr;

            SpawnDesc() = default;
            SpawnDesc(const Engine::Vector3& position, float width = 100.0f, float depth = 100.0f,
                      Engine::PlanarReflectionSystem* reflectionSystem = nullptr,
                      int gridX = 30, int gridZ = 30)
                : m_position(position), m_width(width), m_depth(depth), 
                  m_gridX(gridX), m_gridZ(gridZ), m_reflectionSystem(reflectionSystem) {
            }
        };

        static Engine::GameObject* Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc);
    };

} // namespace Game
