#include "SamplePrefab.h"

#include "Game/Scenes/SceneContext.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/World.h"
#include "Engine/Scene/Entity.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/MeshRenderer.h"

#include "Materials/MaterialLibrary.h"
#include "Materials/SampleCubeMaterial.h"
#include "Game/Scripts/SampleRotateComponent.h"

namespace Game {

    Engine::Entity* SamplePrefab::Spawn(SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_world || !ctx.m_renderSystem || !ctx.m_device) {
            Engine::Logger::Error("SamplePrefab::Spawn failed: ctx invalid.");
            return nullptr;
        }

        auto* e = ctx.m_world->CreateEntity();
        if (!e) return nullptr;

        // Transform 初期値
        if (auto* tr = e->GetComponent<Engine::Transform>()) {
            tr->SetPosition(desc.m_position);
            tr->SetUniformScale(desc.m_uniformScale);
        }

        // MeshRenderer（依存注入は SceneContext から）
        auto* mr = e->AddComponent<Engine::MeshRenderer>(ctx.m_device, ctx.m_renderSystem);
        mr->SetMeshType(Engine::MeshType::Cube);

        // Material（MaterialLibrary から取得）
        if (!ctx.m_materials) {
            Engine::Logger::Error("SamplePrefab::Spawn failed: MaterialLibrary is null.");
            return e;
        }
        mr->SetMaterial(ctx.m_materials->GetOrCreate<SampleCubeMaterial>());

        // 回転挙動
        e->AddComponent<Game::SampleRotateComponent>(desc.m_rotateDegPerSec);

        return e;
    }

} // namespace Game
