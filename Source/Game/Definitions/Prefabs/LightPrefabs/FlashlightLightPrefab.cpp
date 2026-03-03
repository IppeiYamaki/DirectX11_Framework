/// @file   FlashlightLightPrefab.cpp
/// @brief  懐中電灯（SpotLight）Prefab実装
#include "FlashlightLightPrefab.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/LightSystem.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/LightComponent.h"

// 追従コンポーネント
#include "Game/Definitions/Components/LightComponents/FollowTargetLightComponent.h"

namespace Game {

    Engine::GameObject* FlashlightLightPrefab::Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_scene || !ctx.m_lightSystem) {
            Engine::Logger::Error("FlashlightLightPrefab::Spawn failed: ctx invalid.");
            return nullptr;
        }

        // GameObjectを作成
        auto* obj = ctx.m_scene->CreateObject<Engine::GameObject>("FlashlightLight");
        if (!obj) return nullptr;

        // Transformの位置を設定
        if (auto* tr = obj->GetComponent<Engine::Transform>()) {
            tr->SetPosition(desc.m_position);
        }

        // SpotLightComponentを追加
        auto* lightComp = obj->AddComponent<Engine::SpotLightComponent>(
            ctx.m_lightSystem,
            desc.m_direction,
            kDefaultInnerAngle,
            kDefaultOuterAngle
        );

        if (lightComp) {
            // 懐中電灯向けのデフォルトパラメータを適用
            lightComp->SetDirection(desc.m_direction);
            lightComp->SetColor(GetDefaultColor());
            lightComp->SetIntensity(kDefaultIntensity);
            lightComp->SetRange(kDefaultRange);
            lightComp->SetConeAngles(kDefaultInnerAngle, kDefaultOuterAngle);
            lightComp->SetAttenuation(
                kConstantAttenuation,
                kLinearAttenuation,
                kQuadraticAttenuation
            );
        }

        // 追従対象がある場合はFollowTargetLightComponentを追加
        if (desc.m_followTarget) {
            auto* followComp = obj->AddComponent<FollowTargetLightComponent>();
            if (followComp) {
                followComp->SetTarget(desc.m_followTarget);
                followComp->SetOffset(desc.m_followOffset);
                followComp->SetFollowTargetDirection(true);
                Engine::Logger::Info("FlashlightLightPrefab: Following target enabled.");
            }
        }

        Engine::Logger::Info("FlashlightLightPrefab: Spawned at (" +
            std::to_string(desc.m_position.x) + ", " +
            std::to_string(desc.m_position.y) + ", " +
            std::to_string(desc.m_position.z) + ").");

        return obj;
    }

} // namespace Game
