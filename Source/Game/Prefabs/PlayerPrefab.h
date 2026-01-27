/// @file   PlayerPrefab.h
/// @brief  プレイヤーGameObject用のPrefab
#pragma once

#include "Game/Prefabs/GameObjectPrefab.h"
#include "Game/Objects/PlayerObject.h"

namespace Engine { class World; }
namespace Game { struct WorldContext; }

namespace Game {

    /// @brief プレイヤーPrefabの生成設定
    struct PlayerSpawnDesc : public GameObjectSpawnDesc {
        float m_moveSpeed = 5.0f;       ///< 移動速度
        int m_hp = 100;                 ///< 初期HP

        PlayerSpawnDesc() = default;

        PlayerSpawnDesc(const Engine::Vector3& position,
                       float moveSpeed = 5.0f,
                       int hp = 100)
            : GameObjectSpawnDesc(position)
            , m_moveSpeed(moveSpeed)
            , m_hp(hp) {
        }
    };

    /// @brief プレイヤーGameObjectのPrefab
    class PlayerPrefab final {
    public:
        using SpawnDesc = PlayerSpawnDesc;

        /// @brief プレイヤーGameObjectを生成
        /// @param ctx WorldContext
        /// @param desc 生成設定
        /// @return 生成されたPlayerObjectへのポインタ
        static PlayerObject* Spawn(WorldContext& ctx, const SpawnDesc& desc);
    };

} // namespace Game
