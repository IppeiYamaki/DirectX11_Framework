/// @file   Scene.h
/// @brief  Entity群とGameObject群を管理し、Update/Drawを統括するクラス
#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <functional>

#include "Engine/Scene/Entity.h"
#include "Engine/Scene/GameObject.h"

namespace Engine {

    /// @brief Entity群とGameObject群を管理し、Update/Drawを統括する
    /// @note  破棄の安全管理（遅延破棄）は将来的に実装予定
    class Scene final {
    public:
        Scene() = default;
        ~Scene();

        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;

        //============================================================
        // Lifecycle
        //============================================================

        /// @brief  初期化
        /// @return 成功ならtrue
        [[nodiscard]] bool Initialize();

        /// @brief 終了処理
        void Finalize();

        /// @brief リセット（全Entity/GameObjectクリア）
        void Reset();

        //============================================================
        // Frame
        //============================================================

        /// @brief 毎フレーム更新
        /// @param deltaTime フレーム経過時間
        void Update(float deltaTime);

        /// @brief 毎フレーム遅延更新
        /// @param deltaTime フレーム経過時間
        void LateUpdate(float deltaTime);

        /// @brief 毎フレーム描画
        void Draw();

        /// @brief  初期化済みか確認
        /// @return 初期化済みならtrue
        [[nodiscard]] bool IsInitialized() const;

        //============================================================
        // Entity API（既存互換）
        //============================================================

        /// @brief  新しいEntityを生成
        /// @return 生成されたEntity（Transformは自動追加）
        [[nodiscard]] Entity* CreateEntity();

        /// @brief  名前付きEntityを生成
        /// @param  name エンティティ名
        /// @return 生成されたEntity
        [[nodiscard]] Entity* CreateEntity(const std::string& name);

        /// @brief Entityを破棄（即時削除）
        /// @param entity 破棄するEntity
        /// @warning Update中に呼ぶとイテレータ破壊の危険あり
        void DestroyEntity(Entity* entity);

        /// @brief Entityを遅延破棄予約
        /// @param entity 破棄するEntity
        void DestroyEntityDeferred(Entity* entity);

        /// @brief  Entity数を取得
        /// @return Entity数
        [[nodiscard]] std::uint32_t GetEntityCount() const;

        //============================================================
        // GameObject API（新規）
        //============================================================

        /// @brief  GameObjectを追加
        /// @param  object 追加するGameObject（所有権を移譲）
        /// @return 追加されたGameObjectへのポインタ
        GameObject* AddObject(std::unique_ptr<GameObject> object);

        /// @brief  指定した型のGameObjectを生成して追加
        /// @tparam T GameObjectを継承した型
        /// @tparam Args コンストラクタ引数型
        /// @param  args コンストラクタ引数
        /// @return 生成されたGameObjectへのポインタ
        template <class T, class... Args>
        [[nodiscard]] T* CreateObject(Args&&... args) {
            static_assert(std::is_base_of_v<GameObject, T>, "T must derive from GameObject");
            
            auto obj = std::make_unique<T>(std::forward<Args>(args)...);
            T* raw = obj.get();
            
            raw->SetScene(this);
            raw->Initialize();
            
            m_gameObjects.emplace_back(std::move(obj));
            return raw;
        }

        /// @brief GameObjectを破棄（即時削除）
        /// @param object 破棄するGameObject
        /// @warning Update中に呼ぶとイテレータ破壊の危険あり
        void DestroyObject(GameObject* object);

        /// @brief GameObjectを遅延破棄予約
        /// @param object 破棄するGameObject
        void DestroyObjectDeferred(GameObject* object);

        /// @brief  GameObject数を取得
        /// @return GameObject数
        [[nodiscard]] std::uint32_t GetObjectCount() const;

        //============================================================
        // Entity Query
        //============================================================

        /// @brief  名前でEntityを検索
        /// @param  name 検索する名前
        /// @return 見つかったEntity（存在しなければnullptr）
        [[nodiscard]] Entity* FindEntityByName(const std::string& name);

        /// @brief  IDでEntityを検索
        /// @param  id 検索するEntityId
        /// @return 見つかったEntity（存在しなければnullptr）
        [[nodiscard]] Entity* FindEntityById(EntityId id);

        /// @brief  タグを持つEntityを検索
        /// @param  tag 検索するタグ
        /// @return 見つかったEntityのベクター
        [[nodiscard]] std::vector<Entity*> FindEntitiesWithTag(const Tag& tag);

        /// @brief  特定のComponentを持つEntityを検索
        /// @tparam T 検索するComponent型
        /// @return 見つかったEntityのベクター
        template <typename T>
        [[nodiscard]] std::vector<Entity*> FindEntitiesWithComponent() {
            std::vector<Entity*> result;
            for (auto& e : m_entities) {
                if (e->GetComponent<T>() != nullptr) {
                    result.push_back(e.get());
                }
            }
            return result;
        }

        /// @brief  条件を満たすEntityを検索
        /// @param  predicate 検索条件
        /// @return 見つかったEntityのベクター
        template <typename Predicate>
        [[nodiscard]] std::vector<Entity*> FindEntitiesWhere(Predicate predicate) {
            std::vector<Entity*> result;
            for (auto& e : m_entities) {
                if (predicate(e.get())) {
                    result.push_back(e.get());
                }
            }
            return result;
        }

        /// @brief  条件を満たす最初のEntityを検索
        /// @param  predicate 検索条件
        /// @return 見つかったEntity（存在しなければnullptr）
        template <typename Predicate>
        [[nodiscard]] Entity* FindEntityWhere(Predicate predicate) {
            for (auto& e : m_entities) {
                if (predicate(e.get())) {
                    return e.get();
                }
            }
            return nullptr;
        }

        //============================================================
        // GameObject Query
        //============================================================

        /// @brief  名前でGameObjectを検索
        /// @param  name 検索する名前
        /// @return 見つかったGameObject（存在しなければnullptr）
        [[nodiscard]] GameObject* FindObjectByName(const std::string& name);

        /// @brief  IDでGameObjectを検索
        /// @param  id 検索するEntityId
        /// @return 見つかったGameObject（存在しなければnullptr）
        [[nodiscard]] GameObject* FindObjectById(EntityId id);

        /// @brief  タグでGameObjectを検索
        /// @param  tag 検索するタグ
        /// @return 見つかったGameObjectのベクター
        [[nodiscard]] std::vector<GameObject*> FindObjectsWithTag(const std::string& tag);

        /// @brief  特定のComponentを持つGameObjectを検索
        /// @tparam T 検索するComponent型
        /// @return 見つかったGameObjectのベクター
        template <typename T>
        [[nodiscard]] std::vector<GameObject*> FindObjectsWithComponent() {
            std::vector<GameObject*> result;
            for (auto& obj : m_gameObjects) {
                if (obj->GetComponent<T>() != nullptr) {
                    result.push_back(obj.get());
                }
            }
            return result;
        }

        /// @brief  条件を満たすGameObjectを検索
        /// @param  predicate 検索条件
        /// @return 見つかったGameObjectのベクター
        template <typename Predicate>
        [[nodiscard]] std::vector<GameObject*> FindObjectsWhere(Predicate predicate) {
            std::vector<GameObject*> result;
            for (auto& obj : m_gameObjects) {
                if (predicate(obj.get())) {
                    result.push_back(obj.get());
                }
            }
            return result;
        }

        /// @brief  条件を満たす最初のGameObjectを検索
        /// @param  predicate 検索条件
        /// @return 見つかったGameObject（存在しなければnullptr）
        template <typename Predicate>
        [[nodiscard]] GameObject* FindObjectWhere(Predicate predicate) {
            for (auto& obj : m_gameObjects) {
                if (predicate(obj.get())) {
                    return obj.get();
                }
            }
            return nullptr;
        }

    private:
        /// @brief フレーム終了時に遅延破棄を処理
        void ProcessPendingDestructions();

        /// @brief フレーム終了時にGameObjectの遅延破棄を処理
        void ProcessPendingObjectDestructions();

    private:
        bool m_isInitialized = false;                               ///< 初期化済みフラグ

        // Entity管理（既存互換）
        std::vector<std::unique_ptr<Entity>> m_entities;            ///< 所有Entity群
        std::vector<Entity*> m_pendingDestruction;                  ///< 遅延破棄待ちリスト

        // GameObject管理（新規）
        std::vector<std::unique_ptr<GameObject>> m_gameObjects;     ///< 所有GameObject群
        std::vector<GameObject*> m_pendingObjectDestruction;        ///< 遅延破棄待ちリスト
    };

} // namespace Engine
