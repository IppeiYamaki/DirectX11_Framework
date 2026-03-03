/// @file   CampfirePrefab.cpp
/// @brief  焚き火用アニメーションモデル Prefab 実装
#include "CampfirePrefab.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/LightSystem.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/AnimationModel.h"
#include "Engine/Scene/Components/LightComponent.h"

// 揺らぎコンポーネント
#include "Game/Definitions/Components/LightComponents/FireFlickerComponent.h"

// FireLightPrefab（炎・光源オブジェクト）
#include "Game/Definitions/Prefabs/LightPrefabs/FireLightPrefab.h"

namespace Game {

    Engine::GameObject* CampfirePrefab::Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_scene || !ctx.m_lightSystem || !ctx.m_assets || !ctx.m_device || !ctx.m_renderSystem) {
            Engine::Logger::Error("CampfirePrefab::Spawn failed: ctx invalid.");
            return nullptr;
        }

        //============================================================
        // 親オブジェクト（Campfire本体）を作成
        //============================================================
        auto* campfireObj = ctx.m_scene->CreateObject<Engine::GameObject>("Campfire");
        if (!campfireObj) {
            Engine::Logger::Error("CampfirePrefab::Spawn failed: could not create Campfire GameObject.");
            return nullptr;
        }

        // Transformの位置・スケールを設定
        if (auto* tr = campfireObj->GetComponent<Engine::Transform>()) {
            tr->SetPosition(desc.m_position);
            tr->SetUniformScale(desc.m_uniformScale);
        }

        // AnimationModelコンポーネントを追加
        auto* animModel = campfireObj->AddComponent<Engine::AnimationModel>(
            ctx.m_assets, ctx.m_device, ctx.m_renderSystem
        );
        if (animModel) {
            // 焚き火モデルをロード
            if (!animModel->LoadModel(L"Assets/Models/Environment/Campfire/Campfire_anim.fbx")) {
                Engine::Logger::Warn("CampfirePrefab: Failed to load Campfire animation model, trying static model.");
                // アニメーションモデルが無い場合は静的モデルを試す
                animModel->LoadModel(L"Assets/Models/Environment/Campfire/Campfire.fbx");
            }
        }

        //============================================================
        // 子オブジェクト（FireLightPrefab - 炎・光源）を生成
        //============================================================
        {
            // FireLightPrefabの生成記述子を作成
            // 初期位置は原点（親設定後にローカル座標として設定し直す）
            FireLightPrefab::SpawnDesc fireDesc(Engine::Vector3(0.0f, 0.0f, 0.0f));
            fireDesc.m_intensityFlickerRange = desc.m_intensityFlickerRange;
            fireDesc.m_positionFlickerRange = desc.m_positionFlickerRange;
            fireDesc.m_flickerSpeed = desc.m_flickerSpeed;

            // FireLightPrefabをスポーン
            auto* fireObj = ctx.Spawn<FireLightPrefab>(fireDesc);
            if (fireObj) {
                // 親子関係を設定（Campfireの子オブジェクトとして設定）
                auto* fireTransform = fireObj->GetComponent<Engine::Transform>();
                auto* campfireTransform = campfireObj->GetComponent<Engine::Transform>();
                if (fireTransform && campfireTransform) {
                    // 親を設定（これにより座標が親に追従する）
                    fireTransform->SetParent(campfireTransform);
                    // ローカル位置を設定（親からの相対位置）
                    fireTransform->SetPosition(Engine::Vector3(0.0f, desc.m_fireHeight, 0.0f));
                }

                // 光の強度・範囲を設定（必要に応じてカスタマイズ可能）
                if (auto* lightComp = fireObj->GetComponent<Engine::PointLightComponent>()) {
                    lightComp->SetColor(GetDefaultFireColor());
                    lightComp->SetIntensity(kDefaultIntensity);
                    lightComp->SetRange(kDefaultRange);
                    lightComp->SetAttenuation(1.0f, 0.09f, 0.032f);
                }

                // FireFlickerComponentのパラメータを更新
                // （FireLightPrefabは原点を基準位置として初期化したため、ローカル座標に更新）
                if (auto* flickerComp = fireObj->GetComponent<FireFlickerComponent>()) {
                    flickerComp->SetBasePosition(Engine::Vector3(0.0f, desc.m_fireHeight, 0.0f));
                }
            } else {
                Engine::Logger::Warn("CampfirePrefab: Failed to spawn FireLightPrefab child object.");
            }
        }

        Engine::Logger::Info("CampfirePrefab: Spawned at (" +
            std::to_string(desc.m_position.x) + ", " +
            std::to_string(desc.m_position.y) + ", " +
            std::to_string(desc.m_position.z) + ") with FireLightPrefab child.");

        return campfireObj;
    }

} // namespace Game
