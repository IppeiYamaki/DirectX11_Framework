#pragma once

#include "Game/Worlds/IWorld.h"
#include "Game/Worlds/PrefabSlot.h"

namespace Game { struct WorldContext; }

namespace Game {
    /**
     * @brief ワールドのひな型（ベース）
     * - Worldは「演出（ライティング）」「Prefab配置」「遷移管理」を担う
     * - オブジェクトの見た目/動きの設定は Prefab 側で完結させる
     */
    class SampleWorld final : public IWorld {
    public:
        SampleWorld() = default;
        ~SampleWorld() override = default;

        /// @brief ワールド開始処理
        /// @param ctx World利用コンテキスト情報
        void OnEnter(WorldContext& ctx) override;
        /// @brief ワールド終了処理
        /// @param ctx World利用コンテキスト情報
        void OnExit(WorldContext& ctx) override;

        /// @brief 毎フレーム更新処理
        /// @param ctx World利用コンテキスト情報
        /// @param deltaTime 前フレームからの経過時間（秒）
        void Update(WorldContext& ctx, float deltaTime) override;
        /// @brief 毎フレーム描画処理
        /// @param ctx World利用コンテキスト情報
        void Draw(WorldContext& ctx) override;

    private:
        /// @brief ワールド用ライティング設定を適用
        /// @param ctx World利用コンテキスト情報
        void ApplyWorldLighting(WorldContext& ctx);
        /// @brief ワールド内オブジェクトを生成・配置
        /// @brief Worldの開始時にPrefabを並べるだけ
        /// @param ctx World利用コンテキスト情報
        void BuildWorld(WorldContext& ctx);

    private:
        // カメラEntity（ワールド切替時に破棄するため保持しておく）
        Engine::Entity* m_cameraEntity = nullptr;


        // "後で触る対象" は Slot に入れて管理する
        // PrefabSlot m_player;
        // PrefabSlot m_cameraTarget; // 例：追従対象（playerと別でもOK）

        // 複数に増えるなら vector で管理
         std::vector<PrefabSlot> m_characters;


    };

} // namespace Game
