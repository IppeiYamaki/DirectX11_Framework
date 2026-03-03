#include "Sample.h"

#include "Engine/Scene/SceneContext.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/MeshRenderer.h"

#include "Engine/Materials/MaterialLibrary.h"
#include "Engine/Materials/MaterialBuildContext.h"
#include "Game/Definitions/Materials/SampleCubeMaterial.h"
#include "Game/Definitions/Components/SampleRotateComponent.h"

namespace Game {

    Engine::GameObject* Sample::Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_scene || !ctx.m_renderSystem || !ctx.m_device) {
            Engine::Logger::Error("DebugLight::Spawn failed: ctx invalid.");
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
        std::shared_ptr<Engine::Material> material;
        if (ctx.m_materials) {
            material = ctx.m_materials->GetOrCreate<SampleCubeMaterial>();
        } else {
            Engine::Logger::Warn("Sample::Spawn: MaterialLibrary is null, creating fallback material.");
            material = SampleCubeMaterial::Create(Engine::MaterialBuildContext{ ctx.m_device, ctx.m_assets });
        }
        
        if (material) {
            mr->SetMaterial(material);
        } else {
            Engine::Logger::Error("Sample::Spawn: Failed to create material - rendering will not work!");
        }

        // 回転処理
        obj->AddComponent<Game::SampleRotateComponent>(desc.m_rotateDegPerSec);

        return obj;
    }

} // namespace Game
