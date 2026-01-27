#include "SampleScene.h"

#include <DirectXMath.h>

#include "Engine/Scene/SceneContext.h"
#include "Game/Rendering/DefaultLighting.h"

// Engine Components
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/Camera.h"

// Prefabs
#include "Game/Prefabs/MainCameraPrefab.h"
#include "Game/Prefabs/SkyPrefab.h"
#include "Game/Prefabs/SamplePrefab.h"

namespace Game {

    void SampleScene::Initialize(Engine::SceneContext& ctx) {
        ApplySceneLighting(ctx);
        BuildScene(ctx);
    }

    void SampleScene::Finalize(Engine::SceneContext& ctx) {
        // キャラ破棄
        for (auto& slot : m_characters) {
            slot.Destroy(ctx);
        }
        m_characters.clear();

        // カメラ破棄（ワールドを切り替えるたびに残るのを防ぐ）
        if (m_cameraEntity && ctx.m_scene) {
            ctx.m_scene->DestroyEntity(m_cameraEntity);
            m_cameraEntity = nullptr;
        }
    }

    void SampleScene::Update(Engine::SceneContext& ctx, float deltaTime) {
        (void)ctx;
        (void)deltaTime;
    }

    void SampleScene::Render(Engine::SceneContext& ctx) {
        (void)ctx;
    }

    void SampleScene::ApplySceneLighting(Engine::SceneContext& ctx) {
        if (!ctx.m_renderSystem) return;
        ApplyDefaultLighting(*ctx.m_renderSystem);
    }

    void SampleScene::BuildScene(Engine::SceneContext& ctx) {
        if (!ctx.m_scene || !ctx.m_renderSystem) return;

        //========================
        // MainCamera 生成
        //========================
        m_cameraEntity = ctx.SpawnPrefab<MainCameraPrefab>(Engine::Vector3(0, 3, -8), /*yaw*/0.0f, /*pitch*/-5.0f);
       
        //========================
        // Sky 生成
        //========================
        ctx.SpawnPrefab<SkyPrefab>(Engine::Vector3(0, 0, 0), 200.0f);

        //========================
        // その他の Prefab 生成
        //========================
        m_characters.clear();

        m_characters.emplace_back();
        m_characters.back().Spawn<SamplePrefab>(ctx, Engine::Vector3(0, 0, 0), 1.0f, 45.0f);
    }

} // namespace Game
