/// @file   CameraPrefab.cpp
/// @brief  CameraComponentを持つGameObjectを生成するPrefabクラスの実装
#include "CameraPrefab_TitleSceneMain.h"

#include "Engine/Scene/SceneContext.h"
#include "Engine/Core/Logger.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/CameraComponent.h"
#include "Engine/Scene/CameraSystem.h"

// Component
#include "Game/Definitions/Components/CameraComponents/CameraMovementWASD.h"
#include "Game/Definitions/Components/CameraComponents/CameraMovementQE.h"
#include "Game/Definitions/Components/CameraComponents/CameraRotationArrow.h"

namespace Game {

    Engine::GameObject* CameraPrefab_TitleSceneMain::Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_scene || !ctx.m_renderSystem) {
            Engine::Logger::Error("CameraPrefab::Spawn failed: ctx invalid.");
            return nullptr;
        }

        // カメラ用GameObjectを生成
        auto* obj = ctx.m_scene->CreateObject<Engine::GameObject>("Camera");
        if (!obj) {
            Engine::Logger::Error("CameraPrefab::Spawn failed: CreateObject failed.");
            return nullptr;
        }

        // Transform設定
        if (auto* tr = obj->GetComponent<Engine::Transform>()) {
            tr->SetPosition(desc.m_position);
            tr->SetYawPitchRollDegrees(desc.m_yawDeg, desc.m_pitchDeg, desc.m_rollDeg);
        }

        // CameraComponent追加
        auto* camera = obj->AddComponent<Engine::CameraComponent>(ctx.m_renderSystem);
        if (!camera) {
            Engine::Logger::Error("CameraPrefab::Spawn failed: AddComponent<CameraComponent> failed.");
            ctx.m_scene->DestroyObject(obj);
            return nullptr;
        }

        camera->SetPerspective(desc.m_fovYRad, desc.m_aspect, desc.m_nearZ, desc.m_farZ);

        // メインカメラとして設定する場合
        if (desc.m_isMain) {
            camera->SetMain(true);
            
            // CameraSystemが存在する場合は、そちらにも登録
            if (ctx.m_cameraSystem) {
                ctx.m_cameraSystem->SetMainCamera(camera);
            }

            // Tagも設定しておく
            obj->SetTag("MainCamera");
        }

        Engine::Logger::Info("CameraPrefab::Spawn: Camera created at position ("
            + std::to_string(desc.m_position.x) + ", "
            + std::to_string(desc.m_position.y) + ", "
            + std::to_string(desc.m_position.z) + ")");

        //=======================================================================================
        // Componentの追加
        obj->AddComponent<CameraMovementWASD>(2.0f);      // WASD移動
        obj->AddComponent<CameraMovementQE>(2.0f);        // QE上下移動
        obj->AddComponent<CameraRotationArrow>(45.0f);    // 矢印キー回転


        return obj;
    }

    Engine::CameraComponent* CameraPrefab_TitleSceneMain::GetCameraComponent(Engine::GameObject* cameraObject) {
        if (!cameraObject) return nullptr;
        return cameraObject->GetComponent<Engine::CameraComponent>();
    }

} // namespace Game
