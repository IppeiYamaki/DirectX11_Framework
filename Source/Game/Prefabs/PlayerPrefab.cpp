/// @file   PlayerPrefab.cpp
/// @brief  PlayerObjectを生成するPrefabクラスの実装
#include "PlayerPrefab.h"

#include "Engine/Scene/SceneContext.h"
#include "Engine/Core/Logger.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"

#include "Game/GameObjects/PlayerObject.h"

namespace Game {

    Engine::GameObject* PlayerPrefab::SpawnObject(SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_scene) {
            Engine::Logger::Error("PlayerPrefab::SpawnObject failed: ctx.m_scene is null.");
            return nullptr;
        }

        // PlayerObjectを生成
        auto* player = ctx.m_scene->CreateObject<PlayerObject>(desc.m_name);
        if (!player) {
            Engine::Logger::Error("PlayerPrefab::SpawnObject failed: failed to create PlayerObject.");
            return nullptr;
        }

        // Transform設定
        player->SetPosition(desc.m_position);
        player->SetUniformScale(desc.m_uniformScale);

        // プレイヤー固有設定
        player->SetMoveSpeed(desc.m_moveSpeed);
        player->SetHealth(desc.m_health);

        Engine::Logger::Info("PlayerPrefab::SpawnObject: Created player '" + desc.m_name + "'");

        return player;
    }

} // namespace Game
