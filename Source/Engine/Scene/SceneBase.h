#pragma once

#include <memory>
#include <vector>

namespace Game {
    struct SceneContext;
}

namespace Engine {

    class GameObject;

    using SceneContext = Game::SceneContext;

    /**
     * @brief Scene = State（状態/空間）
     *
     * - Initialize/Finalize：遷移時の処理
     * - Update/Render：毎フレーム
     * - GameObject管理：全シーンで共通のGameObject管理機能を提供
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

        //============================================================
        // GameObject管理機能（全シーン共通）
        //============================================================

        /// @brief GameObjectを追加
        /// @param gameObject 追加するGameObject
        void AddGameObject(const std::shared_ptr<GameObject>& gameObject) {
            m_gameObjects.push_back(gameObject);
        }

        /// @brief 全GameObjectの更新処理
        /// @param deltaTime 前フレームからの経過時間（秒）
        void UpdateGameObjects(float deltaTime);

        /// @brief 全GameObjectの描画処理
        void RenderGameObjects();

        /// @brief 全GameObjectをクリア
        void ClearGameObjects() {
            m_gameObjects.clear();
        }

        /// @brief 管理しているGameObject数を取得
        /// @return GameObject数
        [[nodiscard]] std::size_t GetGameObjectCount() const {
            return m_gameObjects.size();
        }

    protected:
        /// @brief SceneBaseが管理するGameObjectコンテナ
        std::vector<std::shared_ptr<GameObject>> m_gameObjects;
    };

} // namespace Engine
