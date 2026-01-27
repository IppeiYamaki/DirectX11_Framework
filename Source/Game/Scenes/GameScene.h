/// @file   GameScene.h
/// @brief  ゲームシーン - Rayシステムを使用した3Dオブジェクト選択の例
#pragma once

#include "Engine/Scene/SceneBase.h"
#include "Game/Scenes/PrefabSlot.h"

#include <vector>

namespace Engine { 
    struct SceneContext;
    class Button;
}

namespace Game {

    /// @brief Rayシステムを使用したゲームシーン
    /// @note  マウスクリックで3DオブジェクトやUIを選択できるサンプルシーン
    class GameScene final : public Engine::SceneBase {
    public:
        GameScene() = default;
        ~GameScene() override = default;

        /// @brief シーン開始処理
        /// @param ctx Scene利用コンテキスト情報
        void Initialize(Engine::SceneContext& ctx) override;

        /// @brief シーン終了処理
        /// @param ctx Scene利用コンテキスト情報
        void Finalize(Engine::SceneContext& ctx) override;

        /// @brief 毎フレーム更新処理
        /// @param ctx Scene利用コンテキスト情報
        /// @param deltaTime 前フレームからの経過時間（秒）
        void Update(Engine::SceneContext& ctx, float deltaTime) override;

        /// @brief 毎フレーム描画処理
        /// @param ctx Scene利用コンテキスト情報
        void Render(Engine::SceneContext& ctx) override;

    private:
        /// @brief シーン用ライティング設定を適用
        /// @param ctx Scene利用コンテキスト情報
        void ApplySceneLighting(Engine::SceneContext& ctx);

        /// @brief シーン内オブジェクトを生成・配置
        /// @param ctx Scene利用コンテキスト情報
        void BuildScene(Engine::SceneContext& ctx);

        /// @brief UI要素を設定
        /// @param ctx Scene利用コンテキスト情報
        void SetupUI(Engine::SceneContext& ctx);

        /// @brief マウスクリック入力を処理
        /// @param ctx Scene利用コンテキスト情報
        void HandleMouseInput(Engine::SceneContext& ctx);

    private:
        // カメラEntity（シーン切替時に破棄するため保持）
        Engine::Entity* m_cameraEntity = nullptr;

        // 選択中のオブジェクト
        Engine::GameObject* m_selectedObject = nullptr;

        // UI要素
        Engine::Button* m_startButton = nullptr;

        // Prefab管理
        std::vector<PrefabSlot> m_sceneObjects;
    };

} // namespace Game
