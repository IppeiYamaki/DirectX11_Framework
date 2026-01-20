#include "MainCameraPrefab.h"

#include "Game/Scenes/SceneContext.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/World.h"
#include "Engine/Scene/Entity.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/Camera.h"

namespace Game {

    Engine::Entity* MainCameraPrefab::Spawn(SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_world || !ctx.m_renderSystem) {
            Engine::Logger::Error("MainCameraPrefab::Spawn failed: ctx invalid.");
            return nullptr;
        }

        auto* e = ctx.m_world->CreateEntity();
        if (!e) return nullptr;

        // Transform
        if (auto* tr = e->GetComponent<Engine::Transform>()) {
            tr->SetPosition(desc.m_position);
            tr->SetYawPitchRollDegrees(desc.m_yawDeg, desc.m_pitchDeg, desc.m_rollDeg);
        }

        // Camera
        auto* cam = e->AddComponent<Engine::Camera>(ctx.m_renderSystem);
        cam->SetMain(desc.m_isMain);
        cam->SetPerspective(desc.m_fovYRad, desc.m_aspect, desc.m_nearZ, desc.m_farZ);

        return e;
    }

} // namespace Game
