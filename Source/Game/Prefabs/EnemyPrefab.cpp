/// @file   EnemyPrefab.cpp
/// @brief  EnemyObjectを生成するPrefabクラスの実装
#include "EnemyPrefab.h"

#include "Engine/Scene/SceneContext.h"
#include "Engine/Core/Logger.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"

#include "Game/GameObjects/EnemyObject.h"

namespace Game {

    Engine::GameObject* EnemyPrefab::SpawnObject(Game::SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_scene) {
            Engine::Logger::Error("EnemyPrefab::SpawnObject failed: ctx.m_scene is null.");
            return nullptr;
        }

        // EnemyObjectを生成
        auto* enemy = ctx.m_scene->CreateObject<EnemyObject>(desc.m_name);
        if (!enemy) {
            Engine::Logger::Error("EnemyPrefab::SpawnObject failed: failed to create EnemyObject.");
            return nullptr;
        }

        // Transform設定
        enemy->SetPosition(desc.m_position);
        enemy->SetUniformScale(desc.m_uniformScale);

        // 敵固有設定
        enemy->SetMoveSpeed(desc.m_moveSpeed);
        enemy->SetHealth(desc.m_health);
        enemy->SetDetectionRange(desc.m_detectionRange);
        enemy->SetAttackRange(desc.m_attackRange);

        Engine::Logger::Info("EnemyPrefab::SpawnObject: Created enemy '" + desc.m_name + "'");

        return enemy;
    }

} // namespace Game
