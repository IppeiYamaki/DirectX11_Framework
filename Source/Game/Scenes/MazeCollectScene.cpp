/// @file   MazeCollectScene.cpp
/// @brief  迷路収集ゲームシーン実装
#include "MazeCollectScene.h"

#include <DirectXMath.h>

#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/Scene/CameraSystem.h"
#include "Engine/Scene/LightSystem.h"
#include "Engine/Scene/Light.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/CameraComponent.h"
#include "Engine/Scene/Components/MeshRenderer.h"
#include "Engine/Graphics/RenderSystem.h"
#include "Engine/UI/Canvas.h"
#include "Engine/Platform/Input.h"
#include "Engine/Core/Logger.h"

// Materials
#include "Engine/Materials/MaterialLibrary.h"
#include "Game/Definitions/Materials/SampleCubeMaterial.h"

// Maze components
#include "Game/Gameplay/Maze/FirstPersonPlayerController.h"
#include "Game/Gameplay/Maze/CollectibleBallComponent.h"
#include "Game/Gameplay/Maze/CollectibleParticleComponent.h"

// Particle components for firefly effect
#include "Game/Definitions/Components/FireflyParticleComponent.h"
#include "Game/Definitions/Components/ParticleRendererComponent.h"
#include "Engine/Materials/ParticleMaterial.h"
#include "Engine/Graphics/RenderLayer.h"

#include "Game/Definitions/Prefabs/SkyPrefab_Night.h"

namespace Game {

    namespace {
        // GameObject* の配列をまとめて破棄するユーティリティ
        void DestroyObjects(Engine::SceneContext& ctx, std::vector<Engine::GameObject*>& objects) {
            if (!ctx.m_scene) {
                objects.clear();
                return;
            }

            for (auto* obj : objects) {
                if (obj) {
                    ctx.m_scene->DestroyObject(obj);
                }
            }
            objects.clear();
        }

        // 単体 GameObject* を安全に破棄するユーティリティ
        void DestroyObject(Engine::SceneContext& ctx, Engine::GameObject*& obj) {
            if (obj && ctx.m_scene) {
                ctx.m_scene->DestroyObject(obj);
            }
            obj = nullptr;
        }
    }

    void MazeCollectScene::Initialize(Engine::SceneContext& ctx) {
        Engine::Logger::Info("MazeCollectScene::Initialize");

        // 設定
        m_settings.m_cellSize = 2.0f;
        m_settings.m_wallHeight = 3.0f;
        m_settings.m_sphereScale = 0.4f;
        m_settings.m_pickupRadius = 1.2f;
        m_settings.m_playerRadius = 0.35f;
        m_settings.m_playerEyeHeight = 1.6f;
        m_settings.m_playerMoveSpeed = 5.0f;
        m_settings.m_mouseSensitivity = 0.15f;

        // 迷路を構築
        BuildMaze(ctx);
        ApplySceneLighting(ctx);
        BuildScene(ctx);
    }

    void MazeCollectScene::Finalize(Engine::SceneContext& ctx) {
        Engine::Logger::Info("MazeCollectScene::Finalize");

        // オブジェクト破棄
        DestroyObjects(ctx, m_wallObjects);
        DestroyObjects(ctx, m_ballObjects);
        DestroyObject(ctx, m_floorObject);

        // Player / Camera
        DestroyObject(ctx, m_playerObject);
        m_playerController = nullptr;

        // NOTE:
        // m_cameraObject は Player と同一なので、ここで別途Destroyすると二重破棄になる可能���がある。
        // 念のためポインタだけ無効化する。
        m_cameraObject = nullptr;
    }

    void MazeCollectScene::Update(Engine::SceneContext& ctx, float deltaTime) {
        // リスタートチェック
        if (m_gameManager.IsCleared()) {
            if (Engine::Input::IsKeyTriggered('R')) {
                RestartScene(ctx);
                return;
            }
        }

        // ゲームマネージャー更新
        m_gameManager.Update(deltaTime);

        // プレイヤー位置を球体に伝播
        if (m_playerController) {
            Engine::Vector3 playerPos = m_playerController->GetPlayerPosition();
            m_gameManager.UpdatePlayerPosition(playerPos);
        }
    }

    void MazeCollectScene::Render(Engine::SceneContext& /*ctx*/) {
        // 特に追加の描画処理なし
    }

    void MazeCollectScene::ApplySceneLighting(Engine::SceneContext& ctx) {
        if (!ctx.m_lightSystem) return;

        // 環境光的なディレクショナルライト
        auto* dirLight = ctx.m_lightSystem->AddDirectionalLight(Engine::Vector3(0.3f, -1.0f, 0.5f));
        if (dirLight) {
            dirLight->SetColor(Engine::Vector3(0.4f, 0.4f, 0.5f));
            dirLight->SetIntensity(0.8f);
            dirLight->SetAmbient(Engine::Vector3(0.15f, 0.15f, 0.2f));
        }
    }

    void MazeCollectScene::BuildScene(Engine::SceneContext& ctx) {
        CreateFloor(ctx);
        CreateWalls(ctx);
        CreatePlayer(ctx);
        CreateBalls(ctx);

        ctx.Spawn<SkyPrefab_Night>(Engine::Vector3(0, 0, 0), 300.0f);
    }

    void MazeCollectScene::BuildMaze(Engine::SceneContext& ctx) {
        (void)ctx;
        m_grid.LoadFixedLayout();
        // Player start cell is defined in m_settings
    }

    void MazeCollectScene::CreateWalls(Engine::SceneContext& ctx) {
        if (!ctx.m_scene || !ctx.m_device || !ctx.m_renderSystem || !ctx.m_materials) {
            return;
        }

        auto material = ctx.m_materials->GetOrCreate<SampleCubeMaterial>();
        float cellSize = m_settings.m_cellSize;
        float wallHeight = m_settings.m_wallHeight;

        // Cubeメッシュの基準サイズは1x1x1なので、wallScaleはcellSizeで良い
        // (MeshCreateDesc::Cubeのデフォルトサイズが1.0)

        for (int z = 0; z < m_grid.GetHeight(); ++z) {
            for (int x = 0; x < m_grid.GetWidth(); ++x) {
                if (m_grid.GetCell(x, z) != MazeCellType::Wall) {
                    continue;
                }

                // 壁オブジェクトを生成
                auto* wallObj = ctx.m_scene->CreateObject<Engine::GameObject>("Wall");
                if (!wallObj) continue;

                // Transform設定
                float worldX, worldZ;
                MazeGrid::GridToWorld(x, z, cellSize, worldX, worldZ);

                auto* transform = wallObj->GetComponent<Engine::Transform>();
                if (transform) {
                    transform->SetPosition(Engine::Vector3(worldX, wallHeight * 0.5f, worldZ));
                    transform->SetScale(Engine::Vector3(cellSize, wallHeight, cellSize));
                }

                // MeshRenderer設定
                auto* renderer = wallObj->AddComponent<Engine::MeshRenderer>(ctx.m_device, ctx.m_renderSystem);
                if (renderer) {
                    renderer->SetMeshDesc(Engine::MeshCreateDesc::Cube(1.0f));
                    renderer->SetMaterial(material);
                }

                // GameObject*として管理
                m_wallObjects.push_back(wallObj);
            }
        }

        Engine::Logger::Info("MazeCollectScene: Created " + std::to_string(m_wallObjects.size()) + " wall objects");
    }

    void MazeCollectScene::CreateFloor(Engine::SceneContext& ctx) {
        if (!ctx.m_scene || !ctx.m_device || !ctx.m_renderSystem || !ctx.m_materials) {
            return;
        }

        auto material = ctx.m_materials->GetOrCreate<SampleCubeMaterial>();
        float cellSize = m_settings.m_cellSize;

        // 床の大きさを計算
        float floorWidth = static_cast<float>(m_grid.GetWidth()) * cellSize;
        float floorDepth = static_cast<float>(m_grid.GetHeight()) * cellSize;

        auto* floorObj = ctx.m_scene->CreateObject<Engine::GameObject>("Floor");
        if (!floorObj) return;

        // Transform設定
        auto* transform = floorObj->GetComponent<Engine::Transform>();
        if (transform) {
            transform->SetPosition(Engine::Vector3(floorWidth * 0.5f, -0.05f, floorDepth * 0.5f));
            transform->SetScale(Engine::Vector3(floorWidth, 0.1f, floorDepth));
        }

        // MeshRenderer設定
        auto* renderer = floorObj->AddComponent<Engine::MeshRenderer>(ctx.m_device, ctx.m_renderSystem);
        if (renderer) {
            renderer->SetMeshDesc(Engine::MeshCreateDesc::Cube(1.0f));
            renderer->SetMaterial(material);
        }

        m_floorObject = floorObj;
    }

    void MazeCollectScene::CreatePlayer(Engine::SceneContext& ctx) {
        if (!ctx.m_scene || !ctx.m_renderSystem || !ctx.m_cameraSystem) {
            return;
        }

        // プレイヤー開始位置を計算
        float worldX, worldZ;
        MazeGrid::GridToWorld(m_settings.m_playerStartCellX, m_settings.m_playerStartCellZ, m_settings.m_cellSize, worldX, worldZ);

        // プレイヤーオブジェクト生成（メッシュなし、見えない）
        m_playerObject = ctx.m_scene->CreateObject<Engine::GameObject>("Player");
        if (!m_playerObject) return;

        // Transform設定
        auto* transform = m_playerObject->GetComponent<Engine::Transform>();
        if (transform) {
            transform->SetPosition(Engine::Vector3(worldX, m_settings.m_playerEyeHeight, worldZ));
        }

        // カメラコンポーネント追加
        auto* camera = m_playerObject->AddComponent<Engine::CameraComponent>(ctx.m_renderSystem);
        if (camera) {
            camera->SetPerspective(DirectX::XM_PIDIV4, 16.0f / 9.0f, 0.1f, 100.0f);
            camera->SetMain(true);

            if (ctx.m_cameraSystem) {
                ctx.m_cameraSystem->SetMainCamera(camera);
            }
            // Tagも設定しておく
            m_playerObject->SetTag("MainCamera");
        }

        // 一人称プレイヤーコントローラー追加
        FirstPersonPlayerSettings playerSettings;
        playerSettings.m_moveSpeed = m_settings.m_playerMoveSpeed;
        playerSettings.m_mouseSensitivity = m_settings.m_mouseSensitivity;
        playerSettings.m_playerRadius = m_settings.m_playerRadius;
        playerSettings.m_eyeHeight = m_settings.m_playerEyeHeight;
        playerSettings.m_pitchLimitMin = -80.0f;
        playerSettings.m_pitchLimitMax = 80.0f;

        m_playerController = m_playerObject->AddComponent<FirstPersonPlayerController>(
            &m_grid, playerSettings, m_settings.m_cellSize);

        m_cameraObject = m_playerObject;  // プレイヤーとカメラは同一オブジェクト

        Engine::Logger::Info("MazeCollectScene: Player created at ("
            + std::to_string(worldX) + ", " + std::to_string(worldZ) + ")");
    }

    void MazeCollectScene::CreateBalls(Engine::SceneContext& ctx) {
        if (!ctx.m_scene || !ctx.m_device || !ctx.m_renderSystem || !ctx.m_materials || !ctx.m_lightSystem) {
            return;
        }

        // パーティクル用マテリアルを取得
        auto particleMaterial = ctx.m_materials->GetOrCreate<Engine::ParticleMaterial>();
        if (!particleMaterial) {
            Engine::Logger::Error("MazeCollectScene::CreateBalls: Failed to create ParticleMaterial");
            return;
        }

        float cellSize = m_settings.m_cellSize;
        float particleY = 1.0f;  // パーティクルの基準高さ

        // パーティクル設定
        // セルからはみ出さない程度のふわふわ範囲
        float driftRangeXZ = cellSize * 0.30f;  // セルサイズの30%
        float driftRangeY = 0.4f;               // 上下の動き
        float particleSize = 0.35f;             // パーティクルサイズ

        // 収集物の数をカウント（プレイヤー開始位置を除く）
        int totalBalls = 0;
        for (int z = 0; z < m_grid.GetHeight(); ++z) {
            for (int x = 0; x < m_grid.GetWidth(); ++x) {
                if (m_grid.GetCell(x, z) == MazeCellType::Floor) {
                    // プレイヤー開始セルはスキップ
                    if (x == m_settings.m_playerStartCellX && z == m_settings.m_playerStartCellZ) {
                        continue;
                    }
                    ++totalBalls;
                }
            }
        }

        // ゲームマネージャー初期化
        m_gameManager.Initialize(ctx.m_canvas, totalBalls);

        // 蛍パーティクル収集物を配置
        for (int z = 0; z < m_grid.GetHeight(); ++z) {
            for (int x = 0; x < m_grid.GetWidth(); ++x) {
                if (m_grid.GetCell(x, z) != MazeCellType::Floor) {
                    continue;
                }

                // プレイヤー開始セルはスキップ
                if (x == m_settings.m_playerStartCellX && z == m_settings.m_playerStartCellZ) {
                    continue;
                }

                // パーティクルオブジェクト生成
                auto* particleObj = ctx.m_scene->CreateObject<Engine::GameObject>("CollectibleParticle");
                if (!particleObj) continue;

                // セル中心のワールド座標を計算
                float worldX, worldZ;
                MazeGrid::GridToWorld(x, z, cellSize, worldX, worldZ);
                Engine::Vector3 basePosition(worldX, particleY, worldZ);

                // Transform設定
                auto* transform = particleObj->GetComponent<Engine::Transform>();
                if (transform) {
                    transform->SetPosition(basePosition);
                }

                // FireflyParticleComponent追加（ふわふわ＋明滅）
                auto* fireflyComp = particleObj->AddComponent<FireflyParticleComponent>();
                if (fireflyComp) {
                    fireflyComp->SetBasePosition(basePosition);
                    fireflyComp->SetDriftRange(Engine::Vector3(driftRangeXZ, driftRangeY, driftRangeXZ));
                    fireflyComp->SetDriftSpeed(0.8f);      // ゆっくり漂う
                    fireflyComp->SetFlickerSpeed(2.0f);    // 明滅速度
                    fireflyComp->SetMinBrightness(0.3f);   // 最小輝度
                    fireflyComp->SetSize(particleSize);
                    // 暖色系の蛍色
                    fireflyComp->SetBaseColor(Engine::Color(1.0f, 0.85f, 0.4f, 1.0f));
                }

                // ParticleRendererComponent追加（Billboard描画）
                auto* rendererComp = particleObj->AddComponent<ParticleRendererComponent>(ctx.m_device, ctx.m_renderSystem);
                if (rendererComp) {
                    rendererComp->SetMaterial(particleMaterial);
                    rendererComp->SetRenderLayer(Engine::RenderLayer::Transparent);
                    rendererComp->SetOrderInLayer(100);
                    // 加算合成で発光効果
                    rendererComp->SetRenderStateFlags(
                        Engine::kRenderStateBlendAdditive |
                        Engine::kRenderStateDepthWriteOff
                    );
                }

                // CollectibleParticleComponent追加（グリッド判定＋PointLight）
                auto* collectComp = particleObj->AddComponent<CollectibleParticleComponent>(
                    &m_gameManager, ctx.m_lightSystem, x, z, cellSize);

                if (collectComp) {
                    // ライトの設定（暖色）
                    collectComp->SetLightColor(Engine::Vector3(1.0f, 0.7f, 0.2f));
                    collectComp->SetLightRange(3.5f);
                    collectComp->SetLightIntensity(1.5f);
                    m_gameManager.RegisterParticle(collectComp);
                }

                // GameObject*として管理
                m_ballObjects.push_back(particleObj);
            }
        }

        Engine::Logger::Info("MazeCollectScene: Created " + std::to_string(totalBalls) + " collectible firefly particles");
    }

    void MazeCollectScene::RestartScene(Engine::SceneContext& ctx) {
        Engine::Logger::Info("MazeCollectScene: Restarting...");

        // オブジェクトを破棄
        DestroyObjects(ctx, m_wallObjects);
        DestroyObjects(ctx, m_ballObjects);
        DestroyObject(ctx, m_floorObject);

        // Player（Cameraも同一）
        DestroyObject(ctx, m_playerObject);
        m_playerController = nullptr;
        m_cameraObject = nullptr;

        // ライトをクリア
        if (ctx.m_lightSystem) {
            ctx.m_lightSystem->ClearLights();
        }

        // UIをクリア
        if (ctx.m_canvas) {
            ctx.m_canvas->ClearElements();
        }

        // シーンを再構築
        m_grid.LoadFixedLayout();
        ApplySceneLighting(ctx);
        BuildScene(ctx);
    }

} // namespace Game