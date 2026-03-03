/// @file   PhysicsSpherePrefab.cpp
/// @brief  物理演算対応Sphere Prefab実装
#include "PhysicsSpherePrefab.h"

#include "Engine/Scene/SceneContext.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/MeshRenderer.h"

#include "Engine/Materials/MaterialLibrary.h"
#include "Engine/Materials/MaterialBuildContext.h"
#include "Engine/Physics/SphereColliderComponent.h"
#include "Game/Definitions/Materials/SampleCubeMaterial.h"

namespace Game {

    Engine::GameObject* PhysicsSpherePrefab::Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_scene || !ctx.m_renderSystem || !ctx.m_device) {
            Engine::Logger::Error("PhysicsSpherePrefab::Spawn failed: ctx invalid.");
            return nullptr;
        }

        auto* obj = ctx.m_scene->CreateObject<Engine::GameObject>(desc.m_name);
        if (!obj) {
            Engine::Logger::Error("PhysicsSpherePrefab::Spawn failed: CreateObject returned nullptr.");
            return nullptr;
        }

        // Transform設定
        if (auto* tr = obj->GetComponent<Engine::Transform>()) {
            tr->SetPosition(desc.m_position);
            tr->SetUniformScale(desc.m_radius * 2.0f); // 直径でスケール
        }

        // MeshRenderer（Sphere）
        auto* mr = obj->AddComponent<Engine::MeshRenderer>(ctx.m_device, ctx.m_renderSystem);
        if (mr) {
            mr->SetMeshDesc(Engine::MeshCreateDesc::SphereUv(0.5f, 32, 16));

            // Material設定
            std::shared_ptr<Engine::Material> material;
            if (ctx.m_materials) {
                material = ctx.m_materials->GetOrCreate<SampleCubeMaterial>();
            } else {
                material = SampleCubeMaterial::Create(Engine::MaterialBuildContext{ ctx.m_device, ctx.m_assets });
            }

            if (material) {
                mr->SetMaterial(material);
            }
        }

        // SphereCollider追加
        auto* collider = obj->AddComponent<Engine::SphereColliderComponent>();
        if (collider) {
            collider->SetDesc(desc.m_colliderDesc);
            collider->SetRadius(desc.m_radius);
        }

        // タグ設定
        obj->SetTag("PhysicsSphere");

        Engine::Logger::Info("PhysicsSpherePrefab: Spawned at (" +
            std::to_string(desc.m_position.x) + ", " +
            std::to_string(desc.m_position.y) + ", " +
            std::to_string(desc.m_position.z) + ") radius=" +
            std::to_string(desc.m_radius) + ", mass=" +
            std::to_string(desc.m_colliderDesc.mass));

        return obj;
    }

} // namespace Game
