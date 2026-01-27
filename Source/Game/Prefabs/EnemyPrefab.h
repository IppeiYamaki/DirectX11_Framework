/// @file   EnemyPrefab.h
/// @brief  敵GameObjectのPrefab
#pragma once

#include "Game/Prefabs/GameObjectPrefab.h"
#include "Game/Objects/EnemyObject.h"

namespace Engine { class World; }
namespace Game { struct WorldContext; }

namespace Game {

    /// @brief 敵Prefabの生成設定
    struct EnemySpawnDesc : public GameObjectSpawnDesc {
        float m_moveSpeed = 3.0f;           ///< 移動速度
        float m_detectionRange = 10.0f;     ///< 検知範囲
        float m_attackRange = 2.0f;         ///< 攻撃範囲
        int m_hp = 50;                      ///< 初期HP
        EnemyBehavior m_initialBehavior = EnemyBehavior::Idle;  ///< 初期振る舞い

        EnemySpawnDesc() = default;

        EnemySpawnDesc(const Engine::Vector3& position,
                      float moveSpeed = 3.0f,
                      float detectionRange = 10.0f,
                      int hp = 50)
            : GameObjectSpawnDesc(position)
            , m_moveSpeed(moveSpeed)
            , m_detectionRange(detectionRange)
            , m_hp(hp) {
        }
    };

    /// @brief 敵GameObjectのPrefab
    class EnemyPrefab final {
    public:
        using SpawnDesc = EnemySpawnDesc;

        /// @brief 敵GameObjectを生成
        /// @param ctx WorldContext
        /// @param desc 生成設定
        /// @return 生成されたEnemyObjectへのポインタ
        static EnemyObject* Spawn(WorldContext& ctx, const SpawnDesc& desc);
    };

} // namespace Game
