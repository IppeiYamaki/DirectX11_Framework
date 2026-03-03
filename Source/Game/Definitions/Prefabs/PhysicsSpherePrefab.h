/// @file   PhysicsSpherePrefab.h
/// @brief  物理演算対応Sphere Prefab
#pragma once
#include <string>

#include "Engine/Math/Vector3.h"
#include "Engine/Physics/ColliderTypes.h"

namespace Engine { 
    struct SceneContext;
    class GameObject;
}

namespace Game {

    /// @brief 物理演算対応SpherePrefab
    /// @note  SphereColliderを持つSphereを生成
    class PhysicsSpherePrefab final {
    public:
        /// @brief 生成パラメータ
        struct SpawnDesc final {
            Engine::Vector3 m_position{ 0, 0, 0 };      ///< 初期位置
            float m_radius = 0.5f;                       ///< 半径
            Engine::ColliderDesc m_colliderDesc;         ///< 物理パラメータ
            std::string m_name = "PhysicsSphere";        ///< オブジェクト名

            SpawnDesc() {
                m_colliderDesc = Engine::ColliderDesc::Dynamic(1.0f);
            }

            /// @brief 位置と半径指定
            SpawnDesc(const Engine::Vector3& position, float radius = 0.5f, float mass = 1.0f)
                : m_position(position)
                , m_radius(radius) {
                m_colliderDesc = Engine::ColliderDesc::Dynamic(mass);
            }

            /// @brief 静的オブジェクト用
            static SpawnDesc Static(const Engine::Vector3& position, float radius = 0.5f) {
                SpawnDesc desc;
                desc.m_position = position;
                desc.m_radius = radius;
                desc.m_colliderDesc = Engine::ColliderDesc::Static();
                return desc;
            }
        };

        /// @brief 物理Sphereを生成してSceneに追加
        /// @param ctx Scene利用コンテキスト
        /// @param desc 生成パラメータ
        /// @return 生成されたGameObject
        static Engine::GameObject* Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc);
    };

} // namespace Game
