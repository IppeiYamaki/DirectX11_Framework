/// @file   LightPrefabs.cpp
/// @brief  ライト用Prefabクラス群の実装
#include "LightPrefabs.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/LightSystem.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/LightComponent.h"

namespace Game {

    //============================================================
    // DirectionalLightPrefab
    //============================================================

    Engine::GameObject* DirectionalLightPrefab::Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_scene || !ctx.m_lightSystem) {
            Engine::Logger::Error("DirectionalLightPrefab::Spawn failed: ctx invalid.");
            return nullptr;
        }

        // GameObjectを作成
        auto* obj = ctx.m_scene->CreateObject<Engine::GameObject>("DirectionalLight");
        if (!obj) return nullptr;

        // LightComponentを追加
        auto* lightComp = obj->AddComponent<Engine::DirectionalLightComponent>(
            ctx.m_lightSystem,
            desc.m_direction
        );

        if (lightComp) {
            lightComp->SetDirection(desc.m_direction);
            lightComp->SetColor(desc.m_color);
            lightComp->SetIntensity(desc.m_intensity);
            lightComp->SetAmbient(desc.m_ambient);
        }

        Engine::Logger::Info("DirectionalLightPrefab: Spawned DirectionalLight GameObject.");
        return obj;
    }

    //============================================================
    // PointLightPrefab
    //============================================================

    Engine::GameObject* PointLightPrefab::Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_scene || !ctx.m_lightSystem) {
            Engine::Logger::Error("PointLightPrefab::Spawn failed: ctx invalid.");
            return nullptr;
        }

        // GameObjectを作成
        auto* obj = ctx.m_scene->CreateObject<Engine::GameObject>("PointLight");
        if (!obj) return nullptr;

        // Transformの位置を設定
        if (auto* tr = obj->GetComponent<Engine::Transform>()) {
            tr->SetPosition(desc.m_position);
        }

        // LightComponentを追加
        auto* lightComp = obj->AddComponent<Engine::PointLightComponent>(ctx.m_lightSystem);

        if (lightComp) {
            lightComp->SetColor(desc.m_color);
            lightComp->SetIntensity(desc.m_intensity);
            lightComp->SetRange(desc.m_range);
            lightComp->SetAttenuation(
                desc.m_constantAttenuation,
                desc.m_linearAttenuation,
                desc.m_quadraticAttenuation
            );
        }

        Engine::Logger::Info("PointLightPrefab: Spawned PointLight GameObject at (" +
            std::to_string(desc.m_position.x) + ", " +
            std::to_string(desc.m_position.y) + ", " +
            std::to_string(desc.m_position.z) + ").");
        return obj;
    }

    //============================================================
    // SpotLightPrefab
    //============================================================

    Engine::GameObject* SpotLightPrefab::Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_scene || !ctx.m_lightSystem) {
            Engine::Logger::Error("SpotLightPrefab::Spawn failed: ctx invalid.");
            return nullptr;
        }

        // GameObjectを作成
        auto* obj = ctx.m_scene->CreateObject<Engine::GameObject>("SpotLight");
        if (!obj) return nullptr;

        // Transformの位置を設定
        if (auto* tr = obj->GetComponent<Engine::Transform>()) {
            tr->SetPosition(desc.m_position);
        }

        // LightComponentを追加
        auto* lightComp = obj->AddComponent<Engine::SpotLightComponent>(
            ctx.m_lightSystem,
            desc.m_direction,
            desc.m_innerAngle,
            desc.m_outerAngle
        );

        if (lightComp) {
            lightComp->SetDirection(desc.m_direction);
            lightComp->SetColor(desc.m_color);
            lightComp->SetIntensity(desc.m_intensity);
            lightComp->SetRange(desc.m_range);
            lightComp->SetConeAngles(desc.m_innerAngle, desc.m_outerAngle);
            lightComp->SetAttenuation(
                desc.m_constantAttenuation,
                desc.m_linearAttenuation,
                desc.m_quadraticAttenuation
            );
        }

        Engine::Logger::Info("SpotLightPrefab: Spawned SpotLight GameObject at (" +
            std::to_string(desc.m_position.x) + ", " +
            std::to_string(desc.m_position.y) + ", " +
            std::to_string(desc.m_position.z) + ").");
        return obj;
    }

} // namespace Game
