#pragma once

#include "Engine/Scene/SceneBase.h"
#include "Engine/Scene/ObjectSlot.h"
#include "Engine/Graphics/PlanarReflectionSystem.h"


#include "Game/GamePlay/TileMapBase.h"
#include <vector>
#include <memory>

namespace Engine { 
    class Button;
}

namespace Game {


    /// @brief タイトルシーン（平面反射付き水面を持つ）
    /// @note  水面に反射するタイトルロゴと蛍パーティクルを表示
    class TitleScene final : public Engine::SceneBase {
    public:
        TitleScene() = default;
        ~TitleScene() override = default;

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

        /// @brief 反射パスを実行
        /// @param ctx Scene利用コンテキスト情報
        void RenderReflectionPass(Engine::SceneContext& ctx);



    private:
        // カメラGameObject（シーン切替時に破棄するため保持）
        Engine::GameObject* m_cameraObject = nullptr;

        // タイトルQuad（3D空間上のTitle.png表示用）
        Engine::GameObject* m_titleQuadObject = nullptr;

        // 水面オブジェクト
        Engine::GameObject* m_waterPlaneObject = nullptr;

        // 平面反射オブジェクト（不可視だがGameObjectとして管理）
        Engine::GameObject* m_planarReflectionObject = nullptr;

        // 蛍パーティクルエミッター
        Engine::GameObject* m_fireflyEmitterObject = nullptr;

        // 選択中のオブジェクト
        Engine::GameObject* m_selectedObject = nullptr;

        // UI要素（Legacy）
        Engine::Button* m_startButton = nullptr;

        // UI要素（Unity-style GameObject）
        Engine::GameObject* m_selectButtonObject = nullptr;

        // GameObject管理
        std::vector<Engine::ObjectSlot> m_sceneObjects;

        // 平面反射システム
        std::unique_ptr<Engine::PlanarReflectionSystem> m_reflectionSystem;

        // シーン遷移フラグ（FadeOut完了後に遷移）
        bool m_isTransitioning = false;

    };

} // namespace Game
