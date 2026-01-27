#pragma once

namespace Game {

    struct WorldContext;
    class WorldManager;

    /**
     * @brief World = State（状態/空間）
     *
     * - OnEnter/OnExit：遷移時の処理（Init/Uninitに相当）
     * - Update/Draw：毎フレーム
     */
    class IWorld {
    public:
        virtual ~IWorld() = default;

        /// @brief ワールド開始処理
        /// @param ctx World利用コンテキスト情報
        virtual void OnEnter(WorldContext& ctx) = 0;
        /// @brief ワールド終了処理
        /// @param ctx World利用コンテキスト情報
        virtual void OnExit(WorldContext& ctx) = 0;

        /// @brief 毎フレーム更新処理
        /// @param ctx World利用コンテキスト情報
        /// @param deltaTime 前フレームからの経過時間（秒）
        virtual void Update(WorldContext& ctx, float deltaTime) = 0;
        /// @brief 毎フレーム描画処理
        /// @param ctx World利用コンテキスト情報
        virtual void Draw(WorldContext& ctx) = 0;
    };

} // namespace Game
