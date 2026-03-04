/// @file   MazeCollectScene.h
/// @brief  迷路収集ゲームシーン
#pragma once

#include "Engine/Scene/SceneBase.h"
#include "Engine/Scene/ObjectSlot.h"
#include "Game/Gameplay/Maze/MazeGrid.h"
#include "Game/Gameplay/Maze/MazeGameManager.h"

#include <vector>
#include <memory>

namespace Game {

    class FirstPersonPlayerController;

    /// @brief 迷路収集ゲームシーン
    class MazeCollectScene final : public Engine::SceneBase {
    public:
        MazeCollectScene() = default;
        ~MazeCollectScene() override = default;

        //============================================================
        // ライフサイクル
        //============================================================

        void Initialize(Engine::SceneContext& ctx) override;
        void Finalize(Engine::SceneContext& ctx) override;
        void Update(Engine::SceneContext& ctx, float deltaTime) override;
        void Render(Engine::SceneContext& ctx) override;

    private:
        //============================================================
        // シーン構築
        //============================================================

        /// @brief ライティング設定を適用
        void ApplySceneLighting(Engine::SceneContext& ctx);

        /// @brief シーンを構築
        void BuildScene(Engine::SceneContext& ctx);

        /// @brief 迷路を構築
        void BuildMaze(Engine::SceneContext& ctx);

        /// @brief 壁を生成
        void CreateWalls(Engine::SceneContext& ctx);

        /// @brief 床を生成
        void CreateFloor(Engine::SceneContext& ctx);

        /// @brief プレイヤーを生成
        void CreatePlayer(Engine::SceneContext& ctx);

        /// @brief 球体を配置
        void CreateBalls(Engine::SceneContext& ctx);

        /// @brief シーンをリセット（リスタート）
        void RestartScene(Engine::SceneContext& ctx);

    private:
        // 迷路データ
        MazeGrid m_grid;
        MazeSettings m_settings;

        // ゲームマネージャー
        MazeGameManager m_gameManager;

        // プレイヤー
        Engine::GameObject* m_playerObject = nullptr;
        FirstPersonPlayerController* m_playerController = nullptr;

        // カメラ
        Engine::GameObject* m_cameraObject = nullptr;

        // シーンオブジェクト
        std::vector<Engine::GameObject*> m_wallObjects;
        std::vector<Engine::GameObject*> m_ballObjects;
        Engine::GameObject* m_floorObject = nullptr;
    };

} // namespace Game
