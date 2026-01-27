#include "SampleScene.h"

#include <DirectXMath.h>

#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/CameraSystem.h"
#include "Game/Rendering/DefaultLighting.h"

// Engine Components
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/Camera.h"

// Physics / Ray
#include "Engine/Physics/Ray.h"
#include "Engine/Physics/Raycast.h"

// UI
#include "Engine/UI/Canvas.h"
#include "Engine/UI/Button.h"

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

        // カメラ破棄（CameraSystemを使用している場合はCameraSystem経由で管理される）
        // 旧来のPrefab方式でカメラを作成した場合のみ、ここで破棄
        if (m_cameraEntity && ctx.m_scene) {
            ctx.m_scene->DestroyEntity(m_cameraEntity);
            m_cameraEntity = nullptr;
        }
    }

    void SampleScene::Update(Engine::SceneContext& ctx, float deltaTime) {
        (void)ctx;
        (void)deltaTime;

        //======================================================================
        // Ray System Usage Example (コメントとして使用例を記載)
        //======================================================================
        //
        // 1. UIクリック処理の例:
        // if (ctx.m_canvas && Input::IsMouseButtonPressed(0)) {
        //     float mouseX = Input::GetMouseX();
        //     float mouseY = Input::GetMouseY();
        //     if (ctx.m_canvas->HandleMouseClick(mouseX, mouseY)) {
        //         // UIがクリックされた
        //         return;
        //     }
        // }
        //
        // 2. 3Dオブジェクトへのレイキャスト例:
        // if (ctx.m_cameraSystem && Input::IsMouseButtonPressed(0)) {
        //     auto* camera = ctx.m_cameraSystem->GetMainCamera();
        //     if (camera) {
        //         float mouseX = Input::GetMouseX();
        //         float mouseY = Input::GetMouseY();
        //         float screenW = ctx.m_canvas->GetScreenWidth();
        //         float screenH = ctx.m_canvas->GetScreenHeight();
        //
        //         Engine::Ray ray = camera->ScreenPointToRay(mouseX, mouseY, screenW, screenH);
        //
        //         Engine::RaycastResult result;
        //         if (Engine::Physics::Raycast(ray, ctx.m_scene, result)) {
        //             // オブジェクトにヒット
        //             auto* hitObject = result.GetHitObject();
        //             auto hitPoint = result.GetHitPoint();
        //             float distance = result.GetDistance();
        //         }
        //     }
        // }
        //======================================================================
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
        // MainCamera 生成（CameraSystemを使用）
        //========================
        if (ctx.m_cameraSystem) {
            // 新しいCameraSystem APIを使用してカメラを作成
            Engine::CameraInitParams cameraParams{};
            cameraParams.m_position = Engine::Vector3(0, 3, -8);
            cameraParams.m_yawDeg = 0.0f;
            cameraParams.m_pitchDeg = -5.0f;
            cameraParams.m_fovYRad = DirectX::XM_PIDIV4;
            cameraParams.m_aspect = 16.0f / 9.0f;
            cameraParams.m_nearZ = 0.1f;
            cameraParams.m_farZ = 1000.0f;
            cameraParams.m_isMain = true;

            auto* mainCamera = ctx.m_cameraSystem->AddCamera(cameraParams);
            ctx.m_cameraSystem->SetMainCamera(mainCamera);
        }
        else {
            // フォールバック：旧来のPrefab方式
            m_cameraEntity = ctx.Spawn<MainCameraPrefab>(Engine::Vector3(0, 3, -8), /*yaw*/0.0f, /*pitch*/-5.0f);
        }

        //========================
        // Canvas UI セットアップ例
        //========================
        if (ctx.m_canvas) {
            // ボタンを追加する例
            // auto* button = ctx.m_canvas->CreateElement<Engine::Button>();
            // button->SetPosition(Engine::Vector2(100.0f, 100.0f));
            // button->SetSize(Engine::Vector2(200.0f, 50.0f));
            // button->SetText(L"Start Game");
            // button->SetOnClick([]() {
            //     // ボタンクリック時の処理
            // });
        }
       
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
