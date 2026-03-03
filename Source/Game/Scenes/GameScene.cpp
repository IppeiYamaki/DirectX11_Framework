#include "GameScene.h"

#include <DirectXMath.h>
#include <iostream>
#include <sstream>

#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/Scene.h"
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

// Prefabs
#include "Game/Definitions/Prefabs/CameraPrefabs/CameraPrefab_GameSceneMain.h"
#include "Game/Definitions/Prefabs/SkyPrefab.h"
#include "Game/Definitions/Prefabs/DebugLight.h"
#include "Game/Definitions/Prefabs/LightPrefabs/AllLightPrefabs.h"
#include "Game/Definitions/Prefabs/FieldPrefab.h"
#include "Game/Definitions/Prefabs/Sample.h"
#include "Game/Definitions/Prefabs/CampfirePrefab.h"

// Gameplay
#include "Game/Gameplay/GridPosition.h"
#include "Game/Gameplay/Rooms/Room_Corridor.h"
#include "Engine/Core/Logger.h"

// Fade System
#include "Engine/Graphics/FadeSystem.h"

namespace Game {

    void GameScene::Initialize(Engine::SceneContext& ctx) {
        ApplySceneLighting(ctx);
        BuildScene(ctx);
        SetupUI(ctx);
        //SetupTileMap(ctx);

        // シーン開始時にFadeIn（画面が見えてくる演出）
        if (ctx.m_fadeSystem) {
            ctx.m_fadeSystem->SetFade(Engine::FadeMode::FadeIn, Engine::EasingType::EaseOutSine, 1.0f);
            Engine::Logger::Info("GameScene: Started FadeIn effect.");
        }
    }

    void GameScene::Finalize(Engine::SceneContext& ctx) {
        // タイルマップ破棄
        m_tileMap.reset();

        // オブジェクト破棄
        for (auto& slot : m_sceneObjects) {
            slot.Destroy(ctx);
        }
        m_sceneObjects.clear();

        // フィールド破棄（SceneのGameObjectとして管理されているが、参照をクリア）
        // Note: SceneのFinalize時に自動的に破棄されるため、
        //       ここでは参照のみクリアする
        m_fieldObject = nullptr;

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

		UpdateTileMapVisuals();

        // マウス入力処理
        HandleMouseInput(ctx);
    }

    void GameScene::Render(Engine::SceneContext& ctx) {
        (void)ctx;
    }



	//============================================================
	// Private用関数
	//============================================================

    void GameScene::ApplySceneLighting(Engine::SceneContext& ctx) {
        if (!ctx.m_renderSystem) return;

        // LightSystemを使用してPrefabでライトを設定
        if (ctx.m_lightSystem) {
            //========================
            // Directional Light（太陽光）をPrefabでスポーン
            //========================
            {
                SunLightPrefab::SpawnDesc sunDesc;
                sunDesc.m_position = Engine::Vector3(0.0f, 100.0f, 0.0f);
                sunDesc.m_direction = Engine::Vector3(45.0f, 180.0f, 45.0f).Normalized();
                sunDesc.m_enableCycle = false;

                auto* sunObj = ctx.Spawn<SunLightPrefab>(sunDesc);
                if (sunObj) {
                    // カスタムパラメータ調整（色・強度・環境光のみ）
                    if (auto* lightComp = sunObj->GetComponent<Engine::DirectionalLightComponent>()) {
                        lightComp->SetColor({ 1.0f, 1.0f, 1.0f });
                        lightComp->SetIntensity(3.0f);
                        lightComp->SetAmbient({ 1.0f, 1.0f, 1.0f });
                    }
                }
            }

            //========================
            // 炎ライト（左側、揺らぎ効果付き）をPrefabでスポーン
            //========================
            {
                FireLightPrefab::SpawnDesc fireDesc(Engine::Vector3(2.5f, 14.0f, 0.0f));
                auto* fireObj = ctx.Spawn<FireLightPrefab>(fireDesc);
                if (fireObj) {
                    if (auto* lightComp = fireObj->GetComponent<Engine::PointLightComponent>()) {
                        lightComp->SetIntensity(1000.0f);
                        lightComp->SetRange(50.0f);
                    }
                    if (auto* flickerComp = fireObj->GetComponent<FireFlickerComponent>()) {
                        flickerComp->SetBaseIntensity(1.0f);
                    }
                }
            }

            Engine::Logger::Info("GameScene: Applied enhanced lighting using LightPrefabs.");
        }
        else {
            // フォールバック：従来のDefaultLightingを使用
            ApplyDefaultLighting(*ctx.m_renderSystem);
        }
    }

    void GameScene::BuildScene(Engine::SceneContext& ctx) {
        if (!ctx.m_scene || !ctx.m_renderSystem) return;

        //========================
        // MainCamera 生成
        //========================
        {
            m_cameraObject = ctx.Spawn<Game::CameraPrefab_GameSceneMain>(
                Engine::Vector3(4.5f, 15.0f, -10.0f),
                0.0f,
                0.0f
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

        //========================
        // Sky 生成
        //========================
        {
            ctx.Spawn<SkyPrefab>(Engine::Vector3(0, 0, 0), 200.0f);
        }

        //========================
        // Field（地形）生成
        //========================
        {
            FieldPrefab::SpawnDesc desc;
            desc.m_position = Engine::Vector3(0.0f, 0.0f, 0.0f);

			// 二つ以上であれば、TerrainBlendが有効になり、テクスチャがブレンドされる
            desc.SetTextures({
                L"Textures/Environment/Field/Ground00.png",
                L"Textures/Environment/Field/Grass.png"
                }, 1.0f, 12.0f);

			// デバッグ用にログ出力
            Engine::Logger::Info("GameScene: Creating Field with m_useTerrainBlend=" +
                std::to_string(desc.m_useTerrainBlend ? 1 : 0) +
                ", m_activeLayerCount=" + std::to_string(desc.m_activeLayerCount) +
                ", m_useMeshRenderer=" + std::to_string(desc.m_useMeshRenderer ? 1 : 0));


            m_fieldObject = ctx.Spawn<FieldPrefab>(desc);
            if (m_fieldObject) {
                Engine::Logger::Info("GameScene: Field created successfully.");
            }
        }

        //========================
		// Sample Object 生成（Prefabを使用して簡単にスポーン）
        //========================
        {
            m_sceneObjects.clear();
			ctx.Spawn<Sample>(Engine::Vector3(2.5f, 14.0f, 0.0f), 1.0f, 0.0f);

			ctx.Spawn<Sample>(Engine::Vector3(5.5f, 17.0f, 0.0f), 1.0f, 20.0f);

			ctx.Spawn<Sample>(Engine::Vector3(7.5f, 15.5f, 0.0f), 1.0f, 45.0f);
        }

        //========================
        // Campfire の生成
        //========================
        {
            CampfirePrefab::SpawnDesc campfireDesc;
            campfireDesc.m_position = Engine::Vector3(0.0f, 2.0f, 0.0f);
            campfireDesc.m_fireHeight = 1.0f;
			campfireDesc.m_intensityFlickerRange = 0.5f;
			campfireDesc.m_positionFlickerRange = 0.2f;
            campfireDesc.m_flickerSpeed = 5.0f;
            ctx.Spawn<CampfirePrefab>(campfireDesc);
		}

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

    void GameScene::SetupTileMap(Engine::SceneContext& ctx) {
        if (!ctx.m_scene || !ctx.m_renderSystem) {
            Engine::Logger::Error("GameScene::SetupTileMap failed: ctx invalid.");
            return;
        }

        m_tileMap = std::make_unique<TileMapBase>(9, 9, 1.0f);
        m_tileMap->SetMapOrigin(Engine::Vector3(0.0f, 0.0f, 0.0f));


        int successCount = 0;
        int placementFailCount = 0;
        int visualFailCount = 0;

        for (int y = 0; y < m_tileMap->GetHeight(); ++y) {
            for (int x = 0; x < m_tileMap->GetWidth(); ++x) {
                GridPosition gridPos(x, y);
                auto corridor = std::make_shared<Room_Corridor>();
                corridor->SetGridPosition(gridPos);

                bool placed = m_tileMap->PlaceRoom(gridPos, corridor);
                if (!placed) {
                    Engine::Logger::Warn("Failed to place corridor at (" +
                        std::to_string(x) + ", " + std::to_string(y) + ")");
                    placementFailCount++;
                    continue;
                }

                Engine::Vector3 worldPos = m_tileMap->GridToWorldPosition(gridPos);

                if (x == 0 && y == 0) {
                    Engine::Logger::Info("First tile world position: (" +
                        std::to_string(worldPos.x) + ", " +
                        std::to_string(worldPos.y) + ", " +
                        std::to_string(worldPos.z) + ")");
                }

                bool visualCreated = corridor->CreateVisual(ctx, worldPos);
                if (visualCreated) {
                    successCount++;
                } else {
                    visualFailCount++;
                    Engine::Logger::Error("Failed to create visual for tile (" +
                        std::to_string(x) + ", " + std::to_string(y) + ")");
                }
            }
        }

        Engine::Logger::Info("TileMap initialized: " +
            std::to_string(successCount) + " tiles created, " +
            std::to_string(placementFailCount) + " placement failures, " +
            std::to_string(visualFailCount) + " visual failures at origin (" +
            std::to_string(m_tileMap->GetMapOrigin().x) + ", " +
            std::to_string(m_tileMap->GetMapOrigin().y) + ", " +
            std::to_string(m_tileMap->GetMapOrigin().z) + ")");
    }

    void GameScene::UpdateTileMapVisuals() {
        if (!m_tileMap) return;

        // 全タイルを走査してビジュアル位置を更新
        for (int y = 0; y < m_tileMap->GetHeight(); ++y) {
            for (int x = 0; x < m_tileMap->GetWidth(); ++x) {
                GridPosition gridPos(x, y);
                RoomInstance* room = m_tileMap->GetRoom(gridPos);
                
                if (room) {
                    // Room_Corridorにキャスト
                    auto* corridor = dynamic_cast<Room_Corridor*>(room);
                    if (corridor) {
                        // 新しいワールド座標を計算
                        Engine::Vector3 worldPos = m_tileMap->GridToWorldPosition(gridPos);
                        // ビジュアル位置を更新
                        corridor->UpdateVisualPosition(worldPos);
                    }
                }
            }
        }
    }

} // namespace Game
