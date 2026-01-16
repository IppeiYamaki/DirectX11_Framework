#include "SampleScene.h"

#include "Game/Scenes/SceneContext.h"
#include "Game/Rendering/DefaultLighting.h"

// Prefabs
#include "Game/Prefabs/SamplePrefab.h"

namespace Game {

    void SampleScene::OnEnter(SceneContext& ctx) {
        ApplySceneLighting(ctx);
        BuildScene(ctx);
    }

    void SampleScene::OnExit(SceneContext& ctx) {
        (void)ctx;
        m_mainEntity = nullptr;
    }

    void SampleScene::Update(SceneContext& ctx, float deltaTime) {
        (void)ctx;
        (void)deltaTime;

        // Sceneは最小：基本は「遷移入力」だけ置く（必要になったら）
        // 例：
        // if (Engine::Input::IsKeyTriggered(VK_ESCAPE)) { ctx.m_sceneManager->RequestChange(...); }
    }

    void SampleScene::Draw(SceneContext& ctx) {
        (void)ctx;
        // Scene固有UIだけ（今は空でOK）
    }

    void SampleScene::ApplySceneLighting(SceneContext& ctx) {
        if (!ctx.m_renderSystem) return;

        // デフォルトライティング設定を適用（ゲーム側演出）
        ApplyDefaultLighting(*ctx.m_renderSystem);
    }

    void SampleScene::BuildScene(SceneContext& ctx) {
        // Sceneは「Prefabを並べるだけ」
        // Prefabの設定（Mesh/Material/Script等）には触らない

        m_mainEntity = ctx.Spawn<SamplePrefab>(Engine::Vector3(0, 0, 0), 1.0f, 45.0f);
        ctx.Spawn<SamplePrefab>(Engine::Vector3(2, 0, 0), 0.8f, 90.0f);

        ctx.Spawn<SampleRotatingCubePrefab>(Engine::Vector3(0, 0, 0), 1.0f, 45.0f);

    }

} // namespace Game
