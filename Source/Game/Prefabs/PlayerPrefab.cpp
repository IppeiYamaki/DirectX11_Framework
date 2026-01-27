/// @file   PlayerPrefab.cpp
/// @brief  プレイヤーGameObject用のPrefab実装

#include "PlayerPrefab.h"

#include "Game/Worlds/WorldContext.h"
#include "Game/Objects/PlayerObject.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/World.h"
#include "Engine/Scene/GameObject.h"

namespace Game {

    PlayerObject* PlayerPrefab::Spawn(WorldContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_world) {
            Engine::Logger::Error("PlayerPrefab::Spawn failed: World is null.");
            return nullptr;
        }

        // GameObjectとして生成
        auto* player = ctx.m_world->CreateGameObject<PlayerObject>("Player");
        if (!player) {
            Engine::Logger::Error("PlayerPrefab::Spawn failed: CreateGameObject returned null.");
            return nullptr;
        }

        // Transform設定
        player->SetPosition(desc.m_position);
        player->SetRotation(desc.m_rotation);
        player->SetScale(desc.m_scale);

        // Player固有設定
        player->SetMoveSpeed(desc.m_moveSpeed);
        player->SetHP(desc.m_hp);

        Engine::Logger::Info("PlayerPrefab spawned successfully.");
        return player;
    }

} // namespace Game
