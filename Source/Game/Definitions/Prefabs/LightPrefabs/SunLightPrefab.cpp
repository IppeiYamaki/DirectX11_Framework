/// @file   SunLightPrefab.cpp
/// @brief  太陽光（DirectionalLight）Prefab実装
#include "SunLightPrefab.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/LightSystem.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/LightComponent.h"

// 日周変化コンポーネント
#include "Game/Definitions/Components/LightComponents/SunCycleComponent.h"

namespace Game {

    Engine::GameObject* SunLightPrefab::Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_scene || !ctx.m_lightSystem) {
            Engine::Logger::Error("SunLightPrefab::Spawn failed: ctx invalid.");
            return nullptr;
        }

        // GameObjectを作成
        auto* obj = ctx.m_scene->CreateObject<Engine::GameObject>("SunLight");
        if (!obj) return nullptr;

        // DirectionalLightComponentを追加
        auto* lightComp = obj->AddComponent<Engine::DirectionalLightComponent>(
            ctx.m_lightSystem,
            desc.m_direction
        );

        if (lightComp) {
            // 太陽光向けのデフォルトパラメータを適用
            lightComp->SetDirection(desc.m_direction);
            lightComp->SetColor(GetDefaultColor());
            lightComp->SetIntensity(kDefaultIntensity);
            lightComp->SetAmbient(GetDefaultAmbient());
        }

        // 日周変化が有効な場合はSunCycleComponentを追加
        if (desc.m_enableCycle) {
            auto* cycleComp = obj->AddComponent<SunCycleComponent>();
            if (cycleComp) {
                cycleComp->SetDayCycleDuration(desc.m_cycleDuration);
                cycleComp->SetTimeOfDay(desc.m_initialTimeOfDay);
                Engine::Logger::Info("SunLightPrefab: Day cycle enabled (duration: " +
                    std::to_string(desc.m_cycleDuration) + "s).");
            }
        }

        Engine::Logger::Info("SunLightPrefab: Spawned with direction (" +
            std::to_string(desc.m_direction.x) + ", " +
            std::to_string(desc.m_direction.y) + ", " +
            std::to_string(desc.m_direction.z) + ").");


        return obj;
    }

} // namespace Game
