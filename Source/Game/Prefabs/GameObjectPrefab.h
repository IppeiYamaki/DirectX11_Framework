/// @file   GameObjectPrefab.h
/// @brief  GameObjectベースのPrefab基底クラス
#pragma once

#include "Engine/Math/Vector3.h"

#include <memory>

namespace Engine {
    class GameObject;
    class World;
}

namespace Game {

    struct WorldContext;

    /// @brief GameObjectのPrefab生成用基底構成
    struct GameObjectSpawnDesc {
        Engine::Vector3 m_position{ 0, 0, 0 };  ///< 初期位置
        Engine::Vector3 m_rotation{ 0, 0, 0 };  ///< 初期回転（度単位）
        Engine::Vector3 m_scale{ 1, 1, 1 };     ///< 初期スケール

        GameObjectSpawnDesc() = default;

        GameObjectSpawnDesc(const Engine::Vector3& position,
                           const Engine::Vector3& rotation = Engine::Vector3::Zero(),
                           const Engine::Vector3& scale = Engine::Vector3::One())
            : m_position(position)
            , m_rotation(rotation)
            , m_scale(scale) {
        }
    };

    /// @brief GameObjectベースのPrefab基底クラス
    /// @note  このクラスを継承して具体的なPrefabを作成する
    ///        SpawnメソッドでGameObjectを生成し、初期設定を行う
    template <typename TGameObject, typename TSpawnDesc = GameObjectSpawnDesc>
    class GameObjectPrefab {
    public:
        using SpawnDesc = TSpawnDesc;
        using GameObjectType = TGameObject;

        /// @brief GameObjectを生成
        /// @param ctx WorldContext
        /// @param desc 生成設定
        /// @return 生成されたGameObjectへのポインタ
        static TGameObject* Spawn(WorldContext& ctx, const TSpawnDesc& desc);
    };

} // namespace Game
