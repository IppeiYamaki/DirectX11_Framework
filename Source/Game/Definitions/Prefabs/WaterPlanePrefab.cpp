#include "WaterPlanePrefab.h"

#include "Engine/Scene/SceneContext.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/WaterSurfaceComponent.h"

#include "Engine/Graphics/MeshFactory.h"
#include "Engine/Graphics/PlanarReflectionSystem.h"

#include "Engine/Materials/MaterialLibrary.h"
#include "Game/Definitions/Materials/WaterMaterial.h"

namespace Game {

    Engine::GameObject* WaterPlanePrefab::Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_scene || !ctx.m_renderSystem || !ctx.m_device || !ctx.m_materials) {
            Engine::Logger::Error("WaterPlanePrefab::Spawn failed: ctx invalid.");
            return nullptr;
        }

        auto* obj = ctx.m_scene->CreateObject<Engine::GameObject>();
        if (!obj) {
            Engine::Logger::Error("WaterPlanePrefab::Spawn failed: could not create GameObject.");
            return nullptr;
        }

        obj->SetName("WaterPlane");

        // Transform設定（XZ平面なので回転は不要）
        if (auto* tr = obj->GetComponent<Engine::Transform>()) {
            tr->SetPosition(desc.m_position);
            // 水面はXZ平面上にあるため、回転なし
        }

        // WaterSurfaceComponent追加
        auto* waterComp = obj->AddComponent<Engine::WaterSurfaceComponent>(ctx.m_device, ctx.m_renderSystem, ctx.m_assets);
        if (!waterComp) {
            Engine::Logger::Error("WaterPlanePrefab::Spawn failed: could not add WaterSurfaceComponent.");
            ctx.m_scene->DestroyObject(obj);
            return nullptr;
        }

        // 水面メッシュを作成（MeshField：XZ平面の細分割グリッド）
        auto mesh = std::make_shared<Engine::Mesh>();
        Engine::MeshCreateDesc meshDesc = Engine::MeshCreateDesc::Plane(
            desc.m_width, desc.m_depth, desc.m_gridX, desc.m_gridZ);
        if (!Engine::MeshFactory::Create(ctx.m_device, *mesh, meshDesc)) {
            Engine::Logger::Error("WaterPlanePrefab::Spawn failed: could not create water mesh.");
            ctx.m_scene->DestroyObject(obj);
            return nullptr;
        }
        waterComp->SetMesh(mesh);

        // WaterMaterial設定
        auto material = ctx.m_materials->GetOrCreate<Game::WaterMaterial>();
        if (!material) {
            Engine::Logger::Warn("WaterPlanePrefab::Spawn: WaterMaterial creation failed. Water will not render.");
        }
        waterComp->SetMaterial(material);

        // 反射システムを設定
        if (desc.m_reflectionSystem) {
            waterComp->SetReflectionSystem(desc.m_reflectionSystem);
        }

        // 波のパラメータを設定
        waterComp->SetWaveSpeed(1.5f);
        waterComp->SetWaveFrequency(0.3f);
        waterComp->SetWaveAmplitude(0.2f);
        waterComp->SetReflectionDistortion(0.015f);

        Engine::Logger::Info("WaterPlanePrefab: Water plane created at position (" +
            std::to_string(desc.m_position.x) + ", " +
            std::to_string(desc.m_position.y) + ", " +
            std::to_string(desc.m_position.z) + ")");

        return obj;
    }

} // namespace Game
