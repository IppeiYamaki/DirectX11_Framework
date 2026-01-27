#pragma once

namespace Engine {

    struct SceneContext;

    /**
     * @brief Scene = State（状態/空間）
     *
     * - Initialize/Finalize：遷移時の処理
     * - Update/Render：毎フレーム
     */
    class SceneBase {
    public:
        virtual ~SceneBase() = default;

        /// @brief シーン開始処理
        /// @param ctx Scene利用コンテキスト情報
        virtual void Initialize(SceneContext& ctx) = 0;
        /// @brief シーン終了処理
        /// @param ctx Scene利用コンテキスト情報
        virtual void Finalize(SceneContext& ctx) = 0;

        /// @brief 毎フレーム更新処理
        /// @param ctx Scene利用コンテキスト情報
        /// @param deltaTime 前フレームからの経過時間（秒）
        virtual void Update(SceneContext& ctx, float deltaTime) = 0;
        /// @brief 毎フレーム描画処理
        /// @param ctx Scene利用コンテキスト情報
        virtual void Render(SceneContext& ctx) = 0;
    };

} // namespace Engine
