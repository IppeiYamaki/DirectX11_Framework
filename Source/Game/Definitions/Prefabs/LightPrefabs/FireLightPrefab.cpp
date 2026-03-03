/// @file   FireLightPrefab.cpp
/// @brief  炎のライトPrefab実装
#include "FireLightPrefab.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/LightSystem.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/LightComponent.h"

// 揺らぎコンポーネント
#include "Game/Definitions/Components/LightComponents/FireFlickerComponent.h"

namespace Game {

    Engine::GameObject* FireLightPrefab::Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_scene || !ctx.m_lightSystem) {
            Engine::Logger::Error("FireLightPrefab::Spawn failed: ctx invalid.");
            return nullptr;
        }

        // GameObjectを作成
        auto* obj = ctx.m_scene->CreateObject<Engine::GameObject>("FireLight");
        if (!obj) return nullptr;

        // Transformの位置を設定
        if (auto* tr = obj->GetComponent<Engine::Transform>()) {
            tr->SetPosition(desc.m_position);
        }

        // PointLightComponentを追加
        auto* lightComp = obj->AddComponent<Engine::PointLightComponent>(ctx.m_lightSystem);

        if (lightComp) {
            // 炎向けのデフォルトパラメータを適用
            lightComp->SetColor(GetDefaultColor());
            lightComp->SetIntensity(kDefaultIntensity);
            lightComp->SetRange(kDefaultRange);
            lightComp->SetAttenuation(
                kConstantAttenuation,
                kLinearAttenuation,
                kQuadraticAttenuation
            );
        }

        // FireFlickerComponentを追加（揺らぎ効果）
        auto* flickerComp = obj->AddComponent<FireFlickerComponent>();
        if (flickerComp) {
            flickerComp->SetBasePosition(desc.m_position);
            flickerComp->SetBaseColor(GetDefaultColor());
            flickerComp->SetBaseIntensity(kDefaultIntensity);
            flickerComp->SetIntensityFlickerRange(desc.m_intensityFlickerRange);
            flickerComp->SetPositionFlickerRange(desc.m_positionFlickerRange);
            flickerComp->SetFlickerSpeed(desc.m_flickerSpeed);
        }

        Engine::Logger::Info("FireLightPrefab: Spawned at (" +
            std::to_string(desc.m_position.x) + ", " +
            std::to_string(desc.m_position.y) + ", " +
            std::to_string(desc.m_position.z) + ") with flicker effect.");

        return obj;
    }

} // namespace Game
