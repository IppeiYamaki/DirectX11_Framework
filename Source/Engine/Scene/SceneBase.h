#pragma once

#include <memory>
#include <vector>

#include "Engine/Scene/ObjectSlot.h"

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
     * 
     * @note GameObjectの所有権について:
     *       SceneBaseが管理するm_gameObjectsはshared_ptrを使用しています。
     *       これはSceneBase内での軽量なGameObject管理を目的としており、
     *       複雑なライフサイクル管理が必要な場合はScene::CreateObject()を使用してください。
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
        /// @param gameObject 追加するGameObject（shared_ptrで所有権を共有）
        /// @note  追加されたGameObjectはUpdateGameObjects/RenderGameObjectsで
        ///        自動的に更新・描画されます
        void AddGameObject(const std::shared_ptr<GameObject>& gameObject) {
            m_objectSlot.Add(gameObject);
        }

        /// @brief 全GameObjectの更新処理
        /// @param deltaTime 前フレームからの経過時間（秒）
        /// @note  派生クラスのUpdate()内で呼び出すことで、
        ///        管理している全てのアクティブなGameObjectを更新します。
        ///        これはGameObject::Update()を呼び出す軽量な更新処理です。
        ///        複雑なライフサイクル管理が必要な場合はScene経由での管理を推奨します。
        void UpdateGameObjects(float deltaTime);

        /// @brief 全GameObjectの描画処理
        /// @note  派生クラスのRender()内で呼び出すことで、
        ///        管理している全てのアクティブなGameObjectを描画します。
        ///        これはGameObject::Render()を呼び出す軽量な描画処理です。
        void RenderGameObjects();

        /// @brief 全GameObjectをクリア
        /// @note  シーン終了時（Finalize）で呼び出すことを推奨します
        void ClearGameObjects() {
            m_objectSlot.Clear();
        }

        /// @brief 管理しているGameObject数を取得
        /// @return GameObject数
        [[nodiscard]] std::size_t GetGameObjectCount() const {
            return m_objectSlot.Count();
        }

    protected:
        /// @brief SceneBaseが管理するGameObjectコレクション
        /// @note  派生クラスから直接アクセス可能ですが、
        ///        通常はAddGameObject()やClearGameObjects()を使用してください
        ObjectSlotCollection m_objectSlot;
    };

} // namespace Engine
