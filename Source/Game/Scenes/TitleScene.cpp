#include "TitleScene.h"

#include <DirectXMath.h>
#include <iostream>
#include <sstream>

#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/Scene/CameraSystem.h"
#include "Engine/Scene/LightSystem.h"
#include "Engine/Scene/Light.h"
#include "Game/Rendering/DefaultLighting.h"

// Engine Components
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/CameraComponent.h"
#include "Engine/Scene/Components/LightComponent.h"

// Physics / Ray
#include "Engine/Physics/Ray.h"
#include "Engine/Physics/Raycast.h"

// UI
#include "Engine/UI/Canvas.h"
#include "Engine/UI/Button.h"
#include "Engine/UI/UIRectTransform.h"
#include "Engine/UI/UIImageComponent.h"
#include "Engine/UI/UIButtonComponent.h"

// Resources
#include "Engine/Resources/AssetManager.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Graphics/GraphicsDevice.h"

// Prefabs
#include "Game/Definitions/Prefabs/CameraPrefabs/CameraPrefab_TitleSceneMain.h"
#include "Game/Definitions/Prefabs/SkyPrefab_Title.h"
#include "Game/Definitions/Prefabs/DebugLight.h"
#include "Game/Definitions/Prefabs/TitleQuadPrefab.h"
#include "Game/Definitions/Prefabs/WaterPlanePrefab.h"
#include "Game/Definitions/Prefabs/FireflyEmitterPrefab.h"
#include "Game/Definitions/Prefabs/LightPrefabs/AllLightPrefabs.h"

// Components
#include "Game/Definitions/Components/FloatMotionComponent.h"

// Gameplay
#include "Game/Gameplay/GridPosition.h"
#include "Game/Gameplay/Rooms/Room_Corridor.h"
#include "Engine/Core/Logger.h"

// Scenes
#include "Game/Scenes/GameScene.h"

// Platform (Input)
#include "Engine/Platform/Input.h"

// Planar Reflection
#include "Engine/Graphics/PlanarReflectionSystem.h"
#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Scene/Components/PlanarReflectionComponent.h"

namespace Game {

    void TitleScene::Initialize(Engine::SceneContext& ctx) {
        // 平面反射システムを初期化
        if (ctx.m_renderSystem) {
            auto* gd = ctx.m_renderSystem->GetGraphicsDevice();
            if (gd) {
                m_reflectionSystem = std::make_unique<Engine::PlanarReflectionSystem>();
                if (!m_reflectionSystem->Initialize(gd, 1024, 1024)) {
                    Engine::Logger::Error("TitleScene: Failed to initialize PlanarReflectionSystem.");
                    m_reflectionSystem.reset();
                }
                else {
                    Engine::Logger::Info("TitleScene: PlanarReflectionSystem initialized.");
                }
            }
        }

        BuildScene(ctx);
        SetupUI(ctx);
        ApplySceneLighting(ctx);
    }

    void TitleScene::Finalize(Engine::SceneContext& ctx) {

        // UI GameObjectをCanvasから解除
        if (m_selectButtonObject && ctx.m_canvas) {
            ctx.m_canvas->RemoveUIObject(m_selectButtonObject);
        }

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

        // タイトルQuad破棄
        if (m_titleQuadObject && ctx.m_scene) {
            ctx.m_scene->DestroyObject(m_titleQuadObject);
            m_titleQuadObject = nullptr;
        }

        // 平面反射GameObject破棄（不可視だがGameObjectとして管理）
        if (m_planarReflectionObject && ctx.m_scene) {
            ctx.m_scene->DestroyObject(m_planarReflectionObject);
            m_planarReflectionObject = nullptr;
        }

        // 水面破棄
        if (m_waterPlaneObject && ctx.m_scene) {
            ctx.m_scene->DestroyObject(m_waterPlaneObject);
            m_waterPlaneObject = nullptr;
        }

        // 蛍パーティクルエミッター破棄
        if (m_fireflyEmitterObject && ctx.m_scene) {
            ctx.m_scene->DestroyObject(m_fireflyEmitterObject);
            m_fireflyEmitterObject = nullptr;
        }

        // SelectButton UI GameObject破棄
        if (m_selectButtonObject && ctx.m_scene) {
            ctx.m_scene->DestroyObject(m_selectButtonObject);
            m_selectButtonObject = nullptr;
        }

        // 平面反射システムを終了
        if (m_reflectionSystem) {
            m_reflectionSystem->Finalize();
            m_reflectionSystem.reset();
        }

        m_selectedObject = nullptr;
        m_startButton = nullptr;
    }

    void TitleScene::Update(Engine::SceneContext& ctx, float deltaTime) {
        (void)deltaTime;


        // マウス入力処理
        HandleMouseInput(ctx);
    }


    void TitleScene::Render(Engine::SceneContext& ctx) {
        // 反射パスを実行
        RenderReflectionPass(ctx);
    }

    //============================================================
    // Private用関数
    //============================================================

    void TitleScene::RenderReflectionPass(Engine::SceneContext& ctx) {
        if (!m_reflectionSystem || !ctx.m_renderSystem) return;

        auto* renderSystem = ctx.m_renderSystem;
        auto* reflectionRT = m_reflectionSystem->GetReflectionTarget();
        if (!reflectionRT) return;

        // 現在のビュー行列を保存
        DirectX::XMFLOAT4X4 savedView = renderSystem->GetViewMatrix();
        DirectX::XMFLOAT4X4 savedProj = renderSystem->GetProjectionMatrix();

        // 反射ビュー行列を計算
        DirectX::XMMATRIX mainView = DirectX::XMLoadFloat4x4(&savedView);
        DirectX::XMMATRIX reflectionView = m_reflectionSystem->ComputeReflectionViewMatrix(mainView);

        // 反射ビュー行列を設定
        DirectX::XMFLOAT4X4 reflectionViewF4x4;
        DirectX::XMStoreFloat4x4(&reflectionViewF4x4, reflectionView);
        renderSystem->SetViewMatrix(reflectionViewF4x4);

        // クリップ平面を設定
        DirectX::XMFLOAT4 clipPlane = m_reflectionSystem->GetWorldSpaceClipPlane();
        renderSystem->SetClipPlane(clipPlane);

        // 反射RTへの描画を開始
        static const float kClearColor[4] = { 0.0f, 0.0f, 0.1f, 1.0f }; // 暗い夜空色
        renderSystem->BeginRenderTarget(reflectionRT, kClearColor);

        // 反射パスでシーンを描画（水面自身は除外）
        renderSystem->FlushRenderItems(true);

        // 反射RTへの描画を終了
        renderSystem->EndRenderTarget();

        // クリップ平面を無効化
        renderSystem->ClearClipPlane();

        // ビュー行列を復元
        renderSystem->SetViewMatrix(savedView);
        renderSystem->SetProjectionMatrix(savedProj);

        Engine::Logger::Trace("TitleScene: Reflection pass completed.");
    }



    void TitleScene::ApplySceneLighting(Engine::SceneContext& ctx) {
        if (!ctx.m_renderSystem) return;

        // LightSystemを使用してPrefabでライトを設定
        if (ctx.m_lightSystem) {
            //========================
            // Directional Light（太陽/月光）をPrefabでスポーン
            //========================
            {

                SunLightPrefab::SpawnDesc sunDesc;
				sunDesc.m_position = Engine::Vector3(0.0f, 100.0f, 0.0f);
                sunDesc.m_direction = Engine::Vector3(45.0f, -180.0f, 0.0f).Normalized();
                sunDesc.m_enableCycle = false;  // タイトル画面では固定

                auto* sunObj = ctx.Spawn<SunLightPrefab>(sunDesc);
                if (sunObj) {
                    // カスタムパラメータ調整（色・強度・環境光のみ）
                    if (auto* lightComp = sunObj->GetComponent<Engine::DirectionalLightComponent>()) {
                        lightComp->SetColor({ 1.0f, 1.0f, 1.0f });
                        lightComp->SetIntensity(0.0f);
                        lightComp->SetAmbient({ 1.0f, 1.0f, 1.0f });
                    }
                }
            }

            //========================
            // 炎ライト（左側、揺らぎ効果付き）をPrefabでスポーン
            //========================
            {
                FireLightPrefab::SpawnDesc fireDesc(Engine::Vector3(0.0f, 8.0f, 30.0f));
                fireDesc.m_flickerSpeed = 8.0f;

                auto* fireObj = ctx.Spawn<FireLightPrefab>(fireDesc);
                if (fireObj) {
                    // カスタムパラメータ調整
                    if (auto* lightComp = fireObj->GetComponent<Engine::PointLightComponent>()) {
                        lightComp->SetIntensity(10.0f);
                        lightComp->SetRange(50.0f);
                    }
                    if (auto* flickerComp = fireObj->GetComponent<FireFlickerComponent>()) {
                        flickerComp->SetBaseIntensity(10.0f);
                    }
                }
            }

            //========================
            // 蛍火ライト（右側、緑色）をPrefabでスポーン
            //========================
            {
                PointLightPrefab::SpawnDesc greenDesc;
                greenDesc.m_position = Engine::Vector3(15.0f, 3.0f, 30.0f);
                greenDesc.m_color = Engine::Vector3(0.4f, 1.0f, 0.4f);  // 緑色
                greenDesc.m_intensity = 1.5f;
                greenDesc.m_range = 15.0f;
                greenDesc.m_constantAttenuation = 1.0f;
                greenDesc.m_linearAttenuation = 0.09f;
                greenDesc.m_quadraticAttenuation = 0.032f;

                ctx.Spawn<PointLightPrefab>(greenDesc);
            }

            //========================
            // シアンライト（中央上）をPrefabでスポーン
            //========================
            {
                PointLightPrefab::SpawnDesc cyanDesc;
                cyanDesc.m_position = Engine::Vector3(0.0f, 12.0f, 35.0f);
                cyanDesc.m_color = Engine::Vector3(0.3f, 0.8f, 1.0f);  // シアン色
                cyanDesc.m_intensity = 1.8f;
                cyanDesc.m_range = 25.0f;
                cyanDesc.m_constantAttenuation = 1.0f;
                cyanDesc.m_linearAttenuation = 0.07f;
                cyanDesc.m_quadraticAttenuation = 0.02f;

                ctx.Spawn<PointLightPrefab>(cyanDesc);
            }

            //========================
            // タイトルスポットライトをPrefabでスポーン
            //========================
            {
                SpotLightPrefab::SpawnDesc spotDesc;
                spotDesc.m_position = Engine::Vector3(0.0f, 20.0f, 10.0f);
                spotDesc.m_direction = Engine::Vector3(0.0f, -0.5f, 0.866f);
                spotDesc.m_color = Engine::Vector3(1.0f, 0.95f, 0.8f);  // 暖かい白色
                spotDesc.m_intensity = 2.5f;
                spotDesc.m_range = 50.0f;
                spotDesc.m_innerAngle = 25.0f;
                spotDesc.m_outerAngle = 40.0f;
                spotDesc.m_constantAttenuation = 1.0f;
                spotDesc.m_linearAttenuation = 0.05f;
                spotDesc.m_quadraticAttenuation = 0.01f;

                ctx.Spawn<SpotLightPrefab>(spotDesc);
            }

            Engine::Logger::Info("TitleScene: Applied enhanced lighting using LightPrefabs.");
        }
        else {
            // フォールバック：従来のDefaultLightingを使用
            ApplyDefaultLighting(*ctx.m_renderSystem);
        }
    }

    void TitleScene::BuildScene(Engine::SceneContext& ctx) {
        if (!ctx.m_scene || !ctx.m_renderSystem) return;

        //========================
        // MainCamera 生成（水面の少し上から見下ろす構成）
        //========================
        {
            // カメラを水面(Y=0)の上に配置し、少し下を向く
            constexpr float kCameraY = 8.0f;        // 水面より上
            constexpr float kCameraZ = -15.0f;      // 手前
            constexpr float kPitchDeg = 0.0f;      // 正面

            m_cameraObject = ctx.Spawn<Game::CameraPrefab_TitleSceneMain>(
                Engine::Vector3(0.0f, kCameraY, kCameraZ),
                0.0f,       // yaw
                kPitchDeg   // pitch（下向き）
            );

            if (m_cameraObject) {
                auto* tr = m_cameraObject->GetComponent<Engine::Transform>();
                if (tr) {
                    auto pos = tr->GetPosition();
                    Engine::Logger::Info("Camera position: (" +
                        std::to_string(pos.x) + ", " +
                        std::to_string(pos.y) + ", " +
                        std::to_string(pos.z) + ")");
                }
            }
        }

        {
            ctx.Spawn<SkyPrefab_Title>(Engine::Vector3(0, 0, 0), 200.0f);

        }


        //========================
        // Title Quad 生成（3D空間上の板ポリゴンにTitle.pngを表示）
        //========================
        {
            constexpr float kTitleQuadX = 0.0f;         // 画面中央（X=0）
            constexpr float kTitleQuadY = 8.0f;         // 水面より上
            constexpr float kTitleQuadZ = 30.0f;        // カメラ前方
            constexpr float kTitleQuadWidth = 20.0f;    // 幅
            constexpr float kTitleQuadHeight = 10.0f;   // 高さ

            m_titleQuadObject = ctx.Spawn<TitleQuadPrefab>(
                Engine::Vector3(kTitleQuadX, kTitleQuadY, kTitleQuadZ),
                kTitleQuadWidth,
                kTitleQuadHeight
            );

            // 浮遊モーションを追加
            if (m_titleQuadObject) {
                auto* floatMotion = m_titleQuadObject->AddComponent<Game::FloatMotionComponent>();
                if (floatMotion) {
                    floatMotion->SetBasePosition(Engine::Vector3(kTitleQuadX, kTitleQuadY, kTitleQuadZ));
                    floatMotion->SetAmplitude(0.5f);
                    floatMotion->SetSpeed(0.8f);
                    floatMotion->SetNoiseStrength(0.1f);
                }
            }
        }


        //========================
        // PlanarReflection 生成（不可視 - 反射システムを管理するGameObject）
        //========================
        {
            constexpr float kWaterY = 0.0f;  // 水面の高さ

            // PlanarReflection GameObjectを作成
            if (m_reflectionSystem && ctx.m_scene) {
                m_planarReflectionObject = ctx.m_scene->CreateEntity("PlanarReflection");
                if (m_planarReflectionObject) {
                    // Transformを水面の位置に設定
                    if (auto* tr = m_planarReflectionObject->GetComponent<Engine::Transform>()) {
                        tr->SetPosition(Engine::Vector3(0.0f, kWaterY, 0.0f));
                    }

                    // PlanarReflectionComponentを追加
                    auto* reflComp = m_planarReflectionObject->AddComponent<Engine::PlanarReflectionComponent>(
                        m_reflectionSystem.get()
                    );
                    if (reflComp) {
                        reflComp->SetPlaneNormal(Engine::Vector3(0.0f, 1.0f, 0.0f));
                        Engine::Logger::Info("TitleScene: PlanarReflection GameObject created.");
                    }
                }
            }
        }

        //========================
        // Water Plane 生成（平面反射付き水面）
        //========================
        {
            constexpr float kWaterY = 0.0f;  // 水面の高さ
            constexpr float kWaterWidth = 200.0f;
            constexpr float kWaterDepth = 200.0f;

            m_waterPlaneObject = ctx.Spawn<WaterPlanePrefab>(
                Engine::Vector3(0.0f, kWaterY, 50.0f),
                kWaterWidth,
                kWaterDepth,
                m_reflectionSystem.get()
            );
        }

        //========================
        // Firefly Particle Emitter 生成（蛍パーティクル群）
        //========================
        {
            constexpr float kWaterY = 0.0f;  // 水面の高さ
            constexpr float kEmitterY = 8.0f;  // エミッターの中心高さ（水面より上）
            constexpr float kEmitterZ = 30.0f;  // カメラ前方

            FireflyEmitterPrefab::SpawnDesc fireflyDesc;
            fireflyDesc.m_position = Engine::Vector3(0.0f, kEmitterY, kEmitterZ);
            fireflyDesc.m_spawnRange = Engine::Vector3(50.0f, 8.0f, 60.0f); // 広範囲に配置
            fireflyDesc.m_particleCount = 150; // 150個の蛍
            fireflyDesc.m_waterSurfaceY = kWaterY;
            fireflyDesc.m_minSize = 0.2f;
            fireflyDesc.m_maxSize = 0.6f;

            m_fireflyEmitterObject = ctx.Spawn<FireflyEmitterPrefab>(fireflyDesc);
        }


    }

    void TitleScene::SetupUI(Engine::SceneContext& ctx) {
        if (!ctx.m_canvas) return;

        //========================
        // Unity-style UI GameObject: SelectButton
        //========================
        if (ctx.m_scene) {
            // GameObjectを作成
            m_selectButtonObject = ctx.m_scene->CreateEntity("SelectButton");

            if (m_selectButtonObject) {
                // UIRectTransformを追加
                auto* rectTransform = m_selectButtonObject->AddComponent<Engine::UIRectTransform>();

                // ボタンの位置とサイズを設定（画面中央下部に配置）
                float screenW = ctx.m_canvas->GetScreenWidth();
                float screenH = ctx.m_canvas->GetScreenHeight();
                constexpr float kButtonWidth = 256.0f;
                constexpr float kButtonHeight = 64.0f;
                float buttonX = (screenW - kButtonWidth) * 0.5f;
                float buttonY = screenH * 0.7f;  // 画面下から30%の位置

                rectTransform->SetPosition(buttonX, buttonY);
                rectTransform->SetSize(kButtonWidth, kButtonHeight);
                rectTransform->SetSortOrder(10);  // 手前に表示

                // UIImageComponentを追加
                auto* imageComp = m_selectButtonObject->AddComponent<Engine::UIImageComponent>();

                // テクスチャをロード (Assets/Textures/UI/Push_Enter.png)
                if (ctx.m_assets) {
                    auto texture = ctx.m_assets->LoadTexture(L"Textures/UI/ClickKey.png");
                    if (texture && texture->IsLoaded()) {
                        imageComp->SetTexture(texture);
                        Engine::Logger::Info("TitleScene: SelectButton UI GameObject - texture loaded successfully.");
                    }
                    else {
                        Engine::Logger::Warn("TitleScene: SelectButton UI GameObject - failed to load texture.");
                    }
                }

                // UIButtonComponentを追加
                auto* buttonComp = m_selectButtonObject->AddComponent<Engine::UIButtonComponent>();

                // クリック時にGameSceneへ遷移
                Engine::SceneManager* sceneManager = ctx.m_sceneManager;
                buttonComp->SetOnClick([sceneManager]() {
                    Engine::Logger::Info("TitleScene: SelectButton (UI GameObject) clicked! Transitioning to GameScene...");
                    if (sceneManager) {
                        sceneManager->ChangeSceneTo<Game::GameScene>();
                    }
                    else {
                        Engine::Logger::Error("TitleScene: SceneManager is null, cannot change scene.");
                    }
                    });

                // CanvasにUI GameObjectを登録
                ctx.m_canvas->AddUIObject(m_selectButtonObject);

                Engine::Logger::Info("TitleScene: SelectButton UI GameObject created and registered with Canvas.");
            }
        }
    }

    void TitleScene::HandleMouseInput(Engine::SceneContext& ctx) {
        // マウス入力を取得
        POINT mousePos = Engine::Input::GetMousePosition();
        float mouseX = static_cast<float>(mousePos.x);
        float mouseY = static_cast<float>(mousePos.y);
        bool isPressed = Engine::Input::IsMousePressed(Engine::Input::MouseButton::Left);

        // UI GameObjectのマウス入力処理（Canvas経由）
        if (ctx.m_canvas) {
            ctx.m_canvas->HandleUIObjectMouseInput(mouseX, mouseY, isPressed);
        }

        // Legacy UIボタンのマウス入力処理（後方互換のため維持）
        if (m_startButton) {
            m_startButton->HandleMouseInput(mouseX, mouseY, isPressed);
        }
    }

} // namespace Game
