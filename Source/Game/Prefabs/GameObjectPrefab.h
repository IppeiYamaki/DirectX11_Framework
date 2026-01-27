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
    /// @note  このクラスは直接使用せず、PlayerPrefabやEnemyPrefabのように
    ///        具体的なPrefabクラスを作成してSpawnメソッドを実装してください。
    ///        SpawnDescはPrefab固有の生成パラメータを定義する構造体です。
    ///
    /// 使用例:
    /// @code
    /// class MyPrefab {
    /// public:
    ///     struct SpawnDesc : public GameObjectSpawnDesc {
    ///         float myParameter = 1.0f;
    ///     };
    ///     static MyGameObject* Spawn(WorldContext& ctx, const SpawnDesc& desc);
    /// };
    /// @endcode
    template <typename TGameObject, typename TSpawnDesc = GameObjectSpawnDesc>
    class GameObjectPrefab {
    public:
        using SpawnDesc = TSpawnDesc;
        using GameObjectType = TGameObject;

        // Note: 具体的なPrefabクラスでstaticメソッドとして実装すること
        // static TGameObject* Spawn(WorldContext& ctx, const TSpawnDesc& desc);
    };

} // namespace Game
