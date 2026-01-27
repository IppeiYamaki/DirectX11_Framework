/// @file   EnemyPrefab.h
/// @brief  EnemyObjectを生成するPrefabクラス
#pragma once

#include "Engine/Math/Vector3.h"
#include <string>

namespace Engine { 
    class GameObject;
}

namespace Game {

    struct SceneContext;
    class EnemyObject;

    /// @brief EnemyObjectを生成するPrefab
    /// @note  GameObjectベースのPrefabパターンの例
    class EnemyPrefab final {
    public:
        /// @brief 生成パラメータ
        struct SpawnDesc final {
            Engine::Vector3 m_position{ 0, 0, 0 };
            float m_uniformScale = 1.0f;
            float m_moveSpeed = 3.0f;
            float m_health = 50.0f;
            float m_detectionRange = 10.0f;
            float m_attackRange = 2.0f;
            std::string m_name = "Enemy";

            SpawnDesc() = default;

            /// @brief 位置指定コンストラクタ
            SpawnDesc(const Engine::Vector3& position,
                      float uniformScale = 1.0f,
                      float moveSpeed = 3.0f,
                      float health = 50.0f,
                      float detectionRange = 10.0f,
                      float attackRange = 2.0f,
                      const std::string& name = "Enemy")
                : m_position(position)
                , m_uniformScale(uniformScale)
                , m_moveSpeed(moveSpeed)
                , m_health(health)
                , m_detectionRange(detectionRange)
                , m_attackRange(attackRange)
                , m_name(name) {
            }
        };

        /// @brief EnemyObjectを生成
        /// @param ctx SceneContext
        /// @param desc 生成パラメータ
        /// @return 生成されたEnemyObject（GameObjectとして返す）
        static Engine::GameObject* SpawnObject(SceneContext& ctx, const SpawnDesc& desc);
    };

} // namespace Game
