#include "SamplePrefab.h"

#include "Engine/Scene/SceneContext.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/MeshRenderer.h"

#include "Materials/MaterialLibrary.h"
#include "Materials/SampleCubeMaterial.h"
#include "Game/Scripts/SampleRotateComponent.h"

namespace Game {

    Engine::GameObject* SamplePrefab::Spawn(Game::SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_scene || !ctx.m_renderSystem || !ctx.m_device) {
            Engine::Logger::Error("SamplePrefab::Spawn failed: ctx invalid.");
            return nullptr;
        }

        auto* obj = ctx.m_scene->CreateObject<Engine::GameObject>();
        if (!obj) return nullptr;

        // Transform 初期値
        if (auto* tr = obj->GetComponent<Engine::Transform>()) {
            tr->SetPosition(desc.m_position);
            tr->SetUniformScale(desc.m_uniformScale);
        }

        // MeshRenderer（依存情報は SceneContext から）
        auto* mr = obj->AddComponent<Engine::MeshRenderer>(ctx.m_device, ctx.m_renderSystem);
        mr->SetMeshType(Engine::MeshType::Cube);

        // Material（MaterialLibrary から取得）
        if (!ctx.m_materials) {
            Engine::Logger::Error("SamplePrefab::Spawn failed: MaterialLibrary is null.");
            return obj;
        }
        mr->SetMaterial(ctx.m_materials->GetOrCreate<SampleCubeMaterial>());

        // 回転処理
        obj->AddComponent<Game::SampleRotateComponent>(desc.m_rotateDegPerSec);

        return obj;
    }

} // namespace Game
