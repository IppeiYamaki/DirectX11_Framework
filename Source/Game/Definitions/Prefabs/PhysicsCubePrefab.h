/// @file   PhysicsCubePrefab.h
/// @brief  物理演算対応Cube Prefab
#pragma once

#include <string>

#include "Engine/Math/Vector3.h"
#include "Engine/Physics/ColliderTypes.h"

namespace Engine { 
    struct SceneContext;
    class GameObject;
}

namespace Game {

    /// @brief 物理演算対応CubePrefab
    /// @note  AABBColliderを持つCubeを生成
    class PhysicsCubePrefab final {
    public:
        /// @brief 生成パラメータ
        struct SpawnDesc final {
            Engine::Vector3 m_position{ 0, 0, 0 };      ///< 初期位置
            float m_uniformScale = 1.0f;                 ///< 均一スケール
            Engine::ColliderDesc m_colliderDesc;         ///< 物理パラメータ
            std::string m_name = "PhysicsCube";          ///< オブジェクト名

            SpawnDesc() {
                m_colliderDesc = Engine::ColliderDesc::Dynamic(1.0f);
            }

            /// @brief 位置とスケール指定
            SpawnDesc(const Engine::Vector3& position, float scale = 1.0f, float mass = 1.0f)
                : m_position(position)
                , m_uniformScale(scale) {
                m_colliderDesc = Engine::ColliderDesc::Dynamic(mass);
            }

            /// @brief 静的オブジェクト用
            static SpawnDesc Static(const Engine::Vector3& position, float scale = 1.0f) {
                SpawnDesc desc;
                desc.m_position = position;
                desc.m_uniformScale = scale;
                desc.m_colliderDesc = Engine::ColliderDesc::Static();
                return desc;
            }
        };

        /// @brief 物理Cubeを生成してSceneに追加
        /// @param ctx Scene利用コンテキスト
        /// @param desc 生成パラメータ
        /// @return 生成されたGameObject
        static Engine::GameObject* Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc);
    };

} // namespace Game
