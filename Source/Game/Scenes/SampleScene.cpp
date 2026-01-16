#include "SampleScene.h"

#include <DirectXMath.h>

#include "Game/Scenes/SceneContext.h"
#include "Game/Rendering/DefaultLighting.h"

// Engine Components
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/Camera.h"

// Prefabs
#include "Game/Prefabs/MainCameraPrefab.h"
#include "Game/Prefabs/SkyPrefab.h"
#include "Game/Prefabs/SamplePrefab.h"

namespace Game {

    void SampleScene::OnEnter(SceneContext& ctx) {
        ApplySceneLighting(ctx);
        BuildScene(ctx);
    }

    void SampleScene::OnExit(SceneContext& ctx) {
        // キャラ破棄
        for (auto& slot : m_characters) {
            slot.Destroy(ctx);
        }
        m_characters.clear();

        // カメラ破棄（シーンを切り替えるたびに残るのを防ぐ）
        if (m_cameraEntity && ctx.m_world) {
            ctx.m_world->DestroyEntity(m_cameraEntity);
            m_cameraEntity = nullptr;
        }
    }

    void SampleScene::Update(SceneContext& ctx, float deltaTime) {
        (void)ctx;
        (void)deltaTime;
    }

    void SampleScene::Draw(SceneContext& ctx) {
        (void)ctx;
    }

    void SampleScene::ApplySceneLighting(SceneContext& ctx) {
        if (!ctx.m_renderSystem) return;
        ApplyDefaultLighting(*ctx.m_renderSystem);
    }

    void SampleScene::BuildScene(SceneContext& ctx) {
        if (!ctx.m_world || !ctx.m_renderSystem) return;

        //========================
        // MainCamera 生成
        //========================
        m_cameraEntity = ctx.Spawn<MainCameraPrefab>(Engine::Vector3(0, 3, -8), /*yaw*/0.0f, /*pitch*/-5.0f);
       
        //========================
		// Sky 生成
        //========================
		ctx.Spawn<SkyPrefab>(Engine::Vector3(0, 0, 0), 200.0f);

        //========================
        // その他の Prefab 生成
        //========================
        m_characters.clear();

        m_characters.emplace_back();
        m_characters.back().Spawn<SamplePrefab>(ctx, Engine::Vector3(0, 0, 0), 1.0f, 45.0f);
    }

} // namespace Game
