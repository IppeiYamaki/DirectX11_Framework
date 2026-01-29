#pragma once

#include "Engine/Scene/SceneBase.h"
#include "Engine/Scene/ObjectSlot.h"

namespace Game {
    /**
     * @brief シーンのひな型（ベース）
     * - Sceneは「演出（ライティング）」「Prefab配置」「遷移管理」を担う
     * - オブジェクトの見た目/動きの設定は Prefab 側で完結させる
     */
    class SampleScene final : public Engine::SceneBase {
    public:
        SampleScene() = default;
        ~SampleScene() override = default;

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
        /// @brief シーン内オブジェクトを生成・配置（Sceneの開始時にPrefabを並べるだけ）
        /// @param ctx Scene利用コンテキスト情報
        void BuildScene(Engine::SceneContext& ctx);

    private:
        // カメラGameObject（シーン切替時に破棄するため保持しておく）
        Engine::GameObject* m_cameraObject = nullptr;

        // "後で触る対象" は Slot に入れて管理する
        // Engine::ObjectSlot m_player;
        // Engine::ObjectSlot m_cameraTarget; // 例：追従対象（playerと別でもOK）

        // 複数に増えるなら vector で管理
        std::vector<Engine::ObjectSlot> m_characters;
    };

} // namespace Game
