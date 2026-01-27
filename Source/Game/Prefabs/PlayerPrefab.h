/// @file   PlayerPrefab.h
/// @brief  PlayerObjectを生成するPrefabクラス
#pragma once

#include "Engine/Math/Vector3.h"
#include <string>

namespace Engine { 
    class GameObject; 
}

namespace Game {

    struct SceneContext;
    class PlayerObject;

    /// @brief PlayerObjectを生成するPrefab
    /// @note  GameObjectベースのPrefabパターンの例
    class PlayerPrefab final {
    public:
        /// @brief 生成パラメータ
        struct SpawnDesc final {
            Engine::Vector3 m_position{ 0, 0, 0 };
            float m_uniformScale = 1.0f;
            float m_moveSpeed = 5.0f;
            float m_health = 100.0f;
            std::string m_name = "Player";

            SpawnDesc() = default;

            /// @brief 位置指定コンストラクタ
            SpawnDesc(const Engine::Vector3& position,
                      float uniformScale = 1.0f,
                      float moveSpeed = 5.0f,
                      float health = 100.0f,
                      const std::string& name = "Player")
                : m_position(position)
                , m_uniformScale(uniformScale)
                , m_moveSpeed(moveSpeed)
                , m_health(health)
                , m_name(name) {
            }
        };

        /// @brief PlayerObjectを生成
        /// @param ctx SceneContext
        /// @param desc 生成パラメータ
        /// @return 生成されたPlayerObject（GameObjectとして返す）
        static Engine::GameObject* SpawnObject(SceneContext& ctx, const SpawnDesc& desc);
    };

} // namespace Game
