#pragma once

#include "Game/Scenes/IScene.h"
#include "Game/Scenes/PrefabSlot.h"

namespace Game { struct SceneContext; }

namespace Game {
    /**
     * @brief シーンのひな型（ベース）
     * - Sceneは「演出（ライト等）」「Prefab配置」「遷移入力」だけ
     * - オブジェクトの見た目/挙動の設定は Prefab 側で完結させる
     */
    class SampleScene final : public IScene {
    public:
        SampleScene() = default;
        ~SampleScene() override = default;

        /// @brief シーン開始時処理
        /// @param ctx Sceneが使うコンテキスト情報
        void OnEnter(SceneContext& ctx) override;
        /// @brief シーン終了時処理
        /// @param ctx Sceneが使うコンテキスト情報
        void OnExit(SceneContext& ctx) override;

        /// @brief 毎フレーム更新処理
        /// @param ctx Sceneが使うコンテキスト情報
        /// @param deltaTime 前フレームからの経過時間（秒）
        void Update(SceneContext& ctx, float deltaTime) override;
        /// @brief 毎フレーム描画処理
        /// @param ctx Sceneが使うコンテキスト情報
        void Draw(SceneContext& ctx) override;

    private:
		/// @brief シーン用ライティング設定を適用
		/// @param ctx Sceneが使うコンテキスト情報
        void ApplySceneLighting(SceneContext& ctx);
		/// @brief シーン内オブジェクトを生成・配置
        /// @brief Sceneの開始時にPrefabを並べるだけ
		/// @param ctx Sceneが使うコンテキスト情報
        void BuildScene(SceneContext& ctx);

    private:
		// カメラEntity（シーン切り替え時に破棄するため保持しておく）
        Engine::Entity* m_cameraEntity = nullptr;


        // “後で触る対象”は Slot に入れて管理する
        // PrefabSlot m_player;
        // PrefabSlot m_cameraTarget; // 例：追従対象（playerと別でもOK）

        // 大量に増えるなら vector で管理
         std::vector<PrefabSlot> m_characters;


    };

} // namespace Game
