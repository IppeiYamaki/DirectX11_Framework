/// @file   PhysicsCubePrefab.cpp
/// @brief  物理演算対応Cube Prefab実装
#include "PhysicsCubePrefab.h"

#include "Engine/Scene/SceneContext.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/MeshRenderer.h"

#include "Engine/Materials/MaterialLibrary.h"
#include "Engine/Materials/MaterialBuildContext.h"
#include "Engine/Physics/AABBColliderComponent.h"
#include "Game/Definitions/Materials/SampleCubeMaterial.h"

namespace Game {

    Engine::GameObject* PhysicsCubePrefab::Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_scene || !ctx.m_renderSystem || !ctx.m_device) {
            Engine::Logger::Error("PhysicsCubePrefab::Spawn failed: ctx invalid.");
            return nullptr;
        }

        auto* obj = ctx.m_scene->CreateObject<Engine::GameObject>(desc.m_name);
        if (!obj) {
            Engine::Logger::Error("PhysicsCubePrefab::Spawn failed: CreateObject returned nullptr.");
            return nullptr;
        }

        // Transform設定
        if (auto* tr = obj->GetComponent<Engine::Transform>()) {
            tr->SetPosition(desc.m_position);
            tr->SetUniformScale(desc.m_uniformScale);
        }

        // MeshRenderer（Cube）
        auto* mr = obj->AddComponent<Engine::MeshRenderer>(ctx.m_device, ctx.m_renderSystem);
        if (mr) {
            mr->SetMeshType(Engine::MeshType::Cube);

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

        // AABBCollider追加
        auto* collider = obj->AddComponent<Engine::AABBColliderComponent>();
        if (collider) {
            collider->SetDesc(desc.m_colliderDesc);
            // Cubeのデフォルトサイズは1x1x1なので、スケールに合わせて半径を設定
            float halfSize = desc.m_uniformScale * 0.5f;
            collider->SetHalfExtents(Engine::Vector3(halfSize, halfSize, halfSize));
        }

        // タグ設定
        obj->SetTag("PhysicsCube");

        Engine::Logger::Info("PhysicsCubePrefab: Spawned at (" +
            std::to_string(desc.m_position.x) + ", " +
            std::to_string(desc.m_position.y) + ", " +
            std::to_string(desc.m_position.z) + ") mass=" +
            std::to_string(desc.m_colliderDesc.mass));

        return obj;
    }

} // namespace Game
