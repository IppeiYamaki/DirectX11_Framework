/// @file   PlayerPrefab_Duck.cpp
/// @brief  アヒル（Duck）モデルを使用したプレイヤーPrefab実装
#include "PlayerPrefab_Duck.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/StaticModelRenderer.h"
#include "Engine/Scene/Components/MeshRenderer.h"

#include "Engine/Materials/MaterialLibrary.h"
#include "Engine/Materials/MaterialBuildContext.h"
#include "Engine/Physics/CapsuleColliderComponent.h"
#include "Game/Definitions/Materials/Characters/PlayerMaterial_Duck.h"

#include "Game/GameObjects/PlayerObject.h"

namespace Game {

    Engine::GameObject* PlayerPrefab_Duck::Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc) {
        // コンテキスト検証
        if (!ctx.m_scene || !ctx.m_assets || !ctx.m_device || !ctx.m_renderSystem) {
            Engine::Logger::Error("PlayerPrefab_Duck::Spawn failed: SceneContext is invalid.");
            return nullptr;
        }

        //============================================================
        // PlayerObjectを生成
        //============================================================
        auto* player = ctx.m_scene->CreateObject<PlayerObject>(desc.m_name);
        if (!player) {
            Engine::Logger::Error("PlayerPrefab_Duck::Spawn failed: Could not create PlayerObject.");
            return nullptr;
        }

        //============================================================
        // Transform設定
        //============================================================
        if (auto* tr = player->GetComponent<Engine::Transform>()) {
            tr->SetPosition(desc.m_position);
            tr->SetUniformScale(desc.m_uniformScale);
        }

        //============================================================
        // プレイヤー固有設定
        //============================================================
        player->SetMoveSpeed(desc.m_moveSpeed);
        player->SetHealth(desc.m_health);

        //============================================================
        // モデルRendererのセットアップ
        //============================================================
        if (!SetupModelRenderer(ctx, player)) {
            Engine::Logger::Warn("PlayerPrefab_Duck::Spawn: Model setup failed, using fallback cube.");
            // フォールバック: MeshRendererでキューブを表示
            auto* mr = player->AddComponent<Engine::MeshRenderer>(ctx.m_device, ctx.m_renderSystem);
            if (mr) {
                mr->SetMeshType(Engine::MeshType::Cube);
                
                // マテリアルを設定
                std::shared_ptr<Engine::Material> material;
                if (ctx.m_materials) {
                    material = ctx.m_materials->GetOrCreate<PlayerMaterial_Duck>();
                } else {
                    material = PlayerMaterial_Duck::Create(Engine::MaterialBuildContext{ ctx.m_device, ctx.m_assets });
                }
                if (material) {
                    mr->SetMaterial(material);
                }
            }
        }

        //============================================================
        // CapsuleCollider追加（プレイヤー用）
        //============================================================
        {
            auto* collider = player->AddComponent<Engine::CapsuleColliderComponent>();
            if (collider) {
                // プレイヤー用の物理設定
                Engine::ColliderDesc physicsDesc;
                physicsDesc.mass = 2.0f;              // プレイヤーの質量
                physicsDesc.isMovable = true;
                physicsDesc.enableGravity = true;
                physicsDesc.friction = 0.8f;          // プレイヤーは摩擦高め
                collider->SetDesc(physicsDesc);

                // カプセルサイズ設定（スケールに合わせる）
                // Duckモデルの概算サイズに合わせる
                float capsuleRadius = 0.3f * desc.m_uniformScale;
                float capsuleHeight = 0.8f * desc.m_uniformScale;
                collider->SetRadius(capsuleRadius);
                collider->SetHeight(capsuleHeight);

                // 中心オフセット（足元を基準にするため上にずらす）
                collider->SetOffset(Engine::Vector3(0, capsuleHeight * 0.5f + capsuleRadius, 0));

                Engine::Logger::Info("PlayerPrefab_Duck: CapsuleCollider added (radius=" +
                    std::to_string(capsuleRadius) + ", height=" + std::to_string(capsuleHeight) + ")");
            }
        }

        Engine::Logger::Info("PlayerPrefab_Duck::Spawn: Created player '" + desc.m_name + "' at (" +
            std::to_string(desc.m_position.x) + ", " +
            std::to_string(desc.m_position.y) + ", " +
            std::to_string(desc.m_position.z) + ")");

        return player;
    }

    bool PlayerPrefab_Duck::SetupModelRenderer(Engine::SceneContext& ctx, Engine::GameObject* obj) {
        if (!obj) return false;

        auto* modelRenderer = obj->AddComponent<Engine::StaticModelRenderer>(ctx.m_assets, ctx.m_device, ctx.m_renderSystem);
        if (!modelRenderer) {
            Engine::Logger::Error("PlayerPrefab_Duck::SetupModelRenderer: Failed to add StaticModelRenderer.");
            return false;
        }

        // Duckモデル＋マテリアル＆テクスチャはMTLで自動セット
        if (!modelRenderer->LoadModel(kDuckModelPath)) {
            Engine::Logger::Warn("PlayerPrefab_Duck::SetupModelRenderer: Failed to load Duck model.");
            return false;
        }

        Engine::Logger::Info("PlayerPrefab_Duck::SetupModelRenderer: Successfully loaded Duck model (materials via .mtl/obj).");
        return true;
    }

} // namespace Game
