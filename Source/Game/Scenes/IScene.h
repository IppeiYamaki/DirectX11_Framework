#pragma once

namespace Game {

    struct SceneContext;
    class SceneManager;

    /**
     * @brief Scene = State（画面/状態）
     *
     * - OnEnter/OnExit：遷移時の処理（ Init/Uninit に相当）
     * - Update/Draw：毎フレーム
     */
    class IScene {
    public:
        virtual ~IScene() = default;

		/// @brief シーン開始時処理
		/// @param ctx Sceneが使うコンテキスト情報
        virtual void OnEnter(SceneContext& ctx) = 0;
		/// @brief シーン終了時処理
		/// @param ctx Sceneが使うコンテキスト情報
        virtual void OnExit(SceneContext& ctx) = 0;

		/// @brief 毎フレーム更新処理
		/// @param ctx Sceneが使うコンテキスト情報
		/// @param deltaTime 前フレームからの経過時間（秒）
        virtual void Update(SceneContext& ctx, float deltaTime) = 0;
		/// @brief 毎フレーム描画処理
		/// @param ctx Sceneが使うコンテキスト情報
        virtual void Draw(SceneContext& ctx) = 0;
    };

} // namespace Game
