#include "SkyPrefab.h"

#include "Game/Scenes/SceneContext.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/World.h"
#include "Engine/Scene/Entity.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/MeshRenderer.h"

#include "Materials/MaterialLibrary.h"
#include "Materials/SkyMaterial.h"

#include "Game/Scripts/SampleRotateComponent.h"

namespace Game {

    Engine::Entity* SkyPrefab::Spawn(SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_world || !ctx.m_renderSystem || !ctx.m_device || !ctx.m_materials) {
            Engine::Logger::Error("SkyDomePrefab::Spawn failed: ctx invalid.");
            return nullptr;
        }

        auto* e = ctx.m_world->CreateEntity();
        if (!e) return nullptr;

        // Transform
        if (auto* tr = e->GetComponent<Engine::Transform>()) {
            tr->SetPosition(desc.m_position);
            tr->SetUniformScale(desc.m_uniformScale);
        }

        // MeshRenderer
        auto* mr = e->AddComponent<Engine::MeshRenderer>(ctx.m_device, ctx.m_renderSystem);
        mr->SetMeshType(Engine::MeshType::Sphere);

        // ★背景として描く
        mr->SetRenderLayer(Engine::RenderLayer::Background);
        mr->SetOrderInLayer(0);

        // ★Sky用ステート（内側表示 + 深度書き込みOFF）
        mr->SetRenderStateFlags(Engine::kRenderStateDepthWriteOff | Engine::kRenderStateCullFront);

        // Material
        mr->SetMaterial(ctx.m_materials->GetOrCreate<Game::SkyMaterial>());

        e->AddComponent<Game::SampleRotateComponent>(0.1f);

        return e;
    }

} // namespace Game
