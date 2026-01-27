/// @file   GameScene.cpp
/// @brief  ゲームシーン実装 - Rayシステムを使用した3Dオブジェクト選択の例
#include "GameScene.h"

#include <DirectXMath.h>
#include <iostream>

#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/CameraSystem.h"
#include "Engine/Scene/LightSystem.h"
#include "Engine/Scene/Light.h"
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
#include "Game/Prefabs/SkyPrefab.h"
#include "Game/Prefabs/SamplePrefab.h"

namespace Game {

    void GameScene::Initialize(Engine::SceneContext& ctx) {
        ApplySceneLighting(ctx);
        BuildScene(ctx);
        SetupUI(ctx);
    }

    void GameScene::Finalize(Engine::SceneContext& ctx) {
        // オブジェクト破棄
        for (auto& slot : m_sceneObjects) {
            slot.Destroy(ctx);
        }
        m_sceneObjects.clear();

        // カメラ破棄
        if (m_cameraObject && ctx.m_scene) {
            ctx.m_scene->DestroyObject(m_cameraObject);
            m_cameraObject = nullptr;
        }

        m_selectedObject = nullptr;
        m_startButton = nullptr;
    }

    void GameScene::Update(Engine::SceneContext& ctx, float deltaTime) {
        (void)deltaTime;

        // マウス入力処理
        HandleMouseInput(ctx);
    }

    void GameScene::Render(Engine::SceneContext& ctx) {
        (void)ctx;
    }

    void GameScene::ApplySceneLighting(Engine::SceneContext& ctx) {
        if (!ctx.m_renderSystem) return;

        // LightSystemを使用してライトを設定
        if (ctx.m_lightSystem) {
            // Directional Light を追加
            auto* dirLight = ctx.m_lightSystem->AddDirectionalLight({0.3f, -1.0f, 0.2f});
            dirLight->SetColor({1.0f, 0.9f, 0.8f});
            dirLight->SetIntensity(1.5f);
            dirLight->SetAmbient({0.2f, 0.2f, 0.2f});

            // Point Light を追加（オプションの例）
            // auto* pointLight = ctx.m_lightSystem->AddPointLight({5.0f, 10.0f, 5.0f});
            // pointLight->SetColor({0.8f, 0.8f, 1.0f});
            // pointLight->SetIntensity(2.0f);
            // pointLight->SetRange(20.0f);

            // Spot Light を追加（オプションの例）
            // auto* spotLight = ctx.m_lightSystem->AddSpotLight({0.0f, 5.0f, -5.0f}, {0.0f, -1.0f, 0.0f}, 30.0f, 40.0f);
            // spotLight->SetColor({1.0f, 1.0f, 0.9f});
            // spotLight->SetIntensity(3.5f);
        }
        else {
            // フォールバック：従来のDefaultLightingを使用
            ApplyDefaultLighting(*ctx.m_renderSystem);
        }
    }

    void GameScene::BuildScene(Engine::SceneContext& ctx) {
        if (!ctx.m_scene || !ctx.m_renderSystem) return;

        //========================
        // MainCamera 生成（CameraSystemを使用）
        //========================
        if (ctx.m_cameraSystem) {
            Engine::CameraInitParams cameraParams{};
            cameraParams.m_position = Engine::Vector3(0, 5, -10);
            cameraParams.m_yawDeg = 0.0f;
            cameraParams.m_pitchDeg = -15.0f;
            cameraParams.m_fovYRad = DirectX::XM_PIDIV4;
            cameraParams.m_aspect = 16.0f / 9.0f;
            cameraParams.m_nearZ = 0.1f;
            cameraParams.m_farZ = 1000.0f;
            cameraParams.m_isMain = true;

            auto* mainCamera = ctx.m_cameraSystem->AddCamera(cameraParams);
            ctx.m_cameraSystem->SetMainCamera(mainCamera);
        }


        //========================
        // Sky 生成
        //========================
        ctx.Spawn<SkyPrefab>(Engine::Vector3(0, 0, 0), 200.0f);

        //========================
        // クリック可能なオブジェクト群を生成
        //========================
        m_sceneObjects.clear();

        // 複数のオブジェクトを配置
        m_sceneObjects.emplace_back();
        m_sceneObjects.back().Spawn<SamplePrefab>(ctx, Engine::Vector3(-3, 0, 0), 1.0f, 0.0f);

        m_sceneObjects.emplace_back();
        m_sceneObjects.back().Spawn<SamplePrefab>(ctx, Engine::Vector3(0, 0, 0), 1.0f, 45.0f);

        m_sceneObjects.emplace_back();
        m_sceneObjects.back().Spawn<SamplePrefab>(ctx, Engine::Vector3(3, 0, 0), 1.0f, 90.0f);
    }

    void GameScene::SetupUI(Engine::SceneContext& ctx) {
        if (!ctx.m_canvas) return;

        //========================
        // スタートボタンを追加
        //========================
        auto button = std::make_unique<Engine::Button>();
        button->SetPosition(Engine::Vector2(100.0f, 100.0f));
        button->SetSize(Engine::Vector2(200.0f, 50.0f));
        button->SetText(L"Start Game");
        button->SetOnClick([]() {
            // ボタンがクリックされたときの処理
            // OutputDebugStringA("Start Button Clicked!\n");
        });

        m_startButton = static_cast<Engine::Button*>(ctx.m_canvas->AddElement(std::move(button)));
    }

    void GameScene::HandleMouseInput(Engine::SceneContext& ctx) {
        // この関数は、実際のInput処理が実装されたら有効化されます
        // 現在はサンプルとして構造のみを提供

        // 使用例:
        // if (!Input::IsMouseButtonPressed(0)) return; // 左クリックチェック
        // float mouseX = Input::GetMouseX();
        // float mouseY = Input::GetMouseY();

        // 1. まずUI要素のクリックをチェック
        // if (ctx.m_canvas) {
        //     if (ctx.m_canvas->HandleMouseClick(mouseX, mouseY)) {
        //         return; // UIがクリックされた場合は3Dオブジェクトの処理をスキップ
        //     }
        // }

        // 2. 3Dオブジェクトへのレイキャスト
        // if (ctx.m_cameraSystem) {
        //     auto* camera = ctx.m_cameraSystem->GetMainCamera();
        //     if (camera && ctx.m_canvas) {
        //         float screenW = ctx.m_canvas->GetScreenWidth();
        //         float screenH = ctx.m_canvas->GetScreenHeight();
        //
        //         Engine::Ray ray = camera->ScreenPointToRay(mouseX, mouseY, screenW, screenH);
        //
        //         Engine::RaycastResult result;
        //         if (Engine::Physics::Raycast(ray, ctx.m_scene, result)) {
        //             m_selectedObject = result.GetHitObject();
        //             // オブジェクトが選択された
        //         } else {
        //             m_selectedObject = nullptr;
        //             // 何も選択されなかった
        //         }
        //     }
        // }
    }

} // namespace Game
