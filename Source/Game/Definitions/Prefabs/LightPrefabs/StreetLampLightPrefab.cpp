/// @file   StreetLampLightPrefab.cpp
/// @brief  街灯用ライトPrefab実装
#include "StreetLampLightPrefab.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/LightSystem.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/LightComponent.h"

namespace Game {

    Engine::GameObject* StreetLampLightPrefab::Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_scene || !ctx.m_lightSystem) {
            Engine::Logger::Error("StreetLampLightPrefab::Spawn failed: ctx invalid.");
            return nullptr;
        }

        // GameObjectを作成
        auto* obj = ctx.m_scene->CreateObject<Engine::GameObject>("StreetLampLight");
        if (!obj) return nullptr;

        // Transformの位置を設定
        if (auto* tr = obj->GetComponent<Engine::Transform>()) {
            tr->SetPosition(desc.m_position);
        }

        // PointLightComponentを追加
        auto* lightComp = obj->AddComponent<Engine::PointLightComponent>(ctx.m_lightSystem);

        if (lightComp) {
            // 街灯向けのデフォルトパラメータを適用
            lightComp->SetColor(GetDefaultColor());
            lightComp->SetIntensity(kDefaultIntensity);
            lightComp->SetRange(kDefaultRange);
            lightComp->SetAttenuation(
                kConstantAttenuation,
                kLinearAttenuation,
                kQuadraticAttenuation
            );
        }

        Engine::Logger::Info("StreetLampLightPrefab: Spawned at (" +
            std::to_string(desc.m_position.x) + ", " +
            std::to_string(desc.m_position.y) + ", " +
            std::to_string(desc.m_position.z) + ").");

        return obj;
    }

} // namespace Game
