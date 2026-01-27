/// @file   EnemyPrefab.cpp
/// @brief  敵GameObjectのPrefab実装

#include "EnemyPrefab.h"

#include "Game/Worlds/WorldContext.h"
#include "Game/Objects/EnemyObject.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/World.h"
#include "Engine/Scene/GameObject.h"

namespace Game {

    EnemyObject* EnemyPrefab::Spawn(WorldContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_world) {
            Engine::Logger::Error("EnemyPrefab::Spawn failed: World is null.");
            return nullptr;
        }

        // GameObjectとして生成
        auto* enemy = ctx.m_world->CreateGameObject<EnemyObject>("Enemy");
        if (!enemy) {
            Engine::Logger::Error("EnemyPrefab::Spawn failed: CreateGameObject returned null.");
            return nullptr;
        }

        // Transform設定
        enemy->SetPosition(desc.m_position);
        enemy->SetRotation(desc.m_rotation);
        enemy->SetScale(desc.m_scale);

        // Enemy固有設定
        enemy->SetMoveSpeed(desc.m_moveSpeed);
        enemy->SetDetectionRange(desc.m_detectionRange);
        enemy->SetAttackRange(desc.m_attackRange);
        enemy->SetHP(desc.m_hp);
        enemy->SetBehavior(desc.m_initialBehavior);

        Engine::Logger::Info("EnemyPrefab spawned successfully.");
        return enemy;
    }

} // namespace Game
