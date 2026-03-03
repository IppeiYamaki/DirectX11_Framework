#include "SkyPrefab.h"

#include "Engine/Scene/SceneContext.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/MeshRenderer.h"

#include "Engine/Materials/MaterialLibrary.h"
#include "Game/Definitions/Materials/SkyMaterial.h"

#include "Game/Definitions/Components/SampleRotateComponent.h"
#include "Game/Definitions/Components/SkyFollowCameraComponent.h"

namespace Game {

    Engine::GameObject* SkyPrefab::Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_scene || !ctx.m_renderSystem || !ctx.m_device || !ctx.m_materials) {
            Engine::Logger::Error("SkyDomePrefab::Spawn failed: ctx invalid.");
            return nullptr;
        }

        auto* obj = ctx.m_scene->CreateObject<Engine::GameObject>();
        if (!obj) return nullptr;

        // Transform
        if (auto* tr = obj->GetComponent<Engine::Transform>()) {
            tr->SetPosition(desc.m_position);
            tr->SetUniformScale(desc.m_uniformScale);
        }

        // MeshRenderer
        auto* mr = obj->AddComponent<Engine::MeshRenderer>(ctx.m_device, ctx.m_renderSystem);
        mr->SetMeshType(Engine::MeshType::Sphere);

        // 背景として描画
        mr->SetRenderLayer(Engine::RenderLayer::Background);
        mr->SetOrderInLayer(0);

        // Sky用ステート（内側表示 + 深度書き込みOFF）
        mr->SetRenderStateFlags(Engine::kRenderStateDepthWriteOff | Engine::kRenderStateCullFront);

        // Material
        mr->SetMaterial(ctx.m_materials->GetOrCreate<Game::SkyMaterial>());


        obj->AddComponent<Game::SampleRotateComponent>(0.01f);
        obj->AddComponent<Game::SkyFollowCameraComponent>();


        return obj;
    }

} // namespace Game
