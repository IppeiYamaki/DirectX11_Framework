/// @file   ObjectSlot.h
/// @brief  GameObjectインスタンス管理用の枠（Engine共通）
#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/Scene.h"

namespace Engine { class GameObject; }

namespace Engine {

    /**
     * @brief GameObjectインスタンス管理用の枠
     * - Sceneで「あとで削除/再生成/追従」したい対象をまとめて扱える
     * - Spawn時の引数を保存し、Respawnで同引数で再生成できる
     *
     * 注意：
     * - Destroy() は Scene::DestroyObject() を呼ぶ（即時削除）
     * - Component Update内など、Scene内からのDestroyしないこと（Scene Update内ならOK）
     */
    class ObjectSlot final {
    public:
        ObjectSlot() = default;

        /// @brief 管理しているGameObjectを取得
        /// @return GameObjectへのポインタ
        GameObject* Get() const { return m_object; }

        /// @brief GameObjectが生存しているか確認
        /// @return 生存していればtrue
        bool IsAlive() const { return m_object != nullptr; }

        /// @brief GameObjectをPrefab経由で生成
        /// @tparam TPrefab SceneContext::Spawn<TPrefab>で生成可能なPrefabクラス
        /// @param ctx SceneContext
        /// @param args Prefab生成用の引数
        /// @return 生成されたGameObject
        template<class TPrefab, class... Args>
        GameObject* Spawn(SceneContext& ctx, Args&&... args) {
            // 引数を値として保持（あとで同引数で再生成するため）
            auto argsTuple = std::make_shared<std::tuple<std::decay_t<Args>...>>(
                std::forward<Args>(args)...);

            // argsTupleは値でキャプチャ、内側のラムダはapplyの引数のみ参照キャプチャ
            m_respawn = [argsTuple](SceneContext& c) -> GameObject* {
                return std::apply(
                    [&c](auto&&... a) -> GameObject* {
                        return c.Spawn<TPrefab>(a...);
                    },
                    *argsTuple);
                };

            // 既に生成してるなら破棄してから生成（転用をシンプルに）
            Destroy(ctx);
            m_object = m_respawn(ctx);
            return m_object;
        }

        /// @brief GameObjectを破棄
        /// @param ctx SceneContext
        void Destroy(SceneContext& ctx) {
            if (!m_object) return;
            if (ctx.m_scene) {
                ctx.m_scene->DestroyObject(m_object);
            }
            m_object = nullptr;
        }

        /// @brief 同じ引数で再生成
        /// @param ctx SceneContext
        /// @return 再生成されたGameObject
        GameObject* Respawn(SceneContext& ctx) {
            Destroy(ctx);
            if (!m_respawn) return nullptr;
            m_object = m_respawn(ctx);
            return m_object;
        }

        /// @brief スロットをクリア（破棄はしない）
        void Clear() {
            m_object = nullptr;
            m_respawn = nullptr;
        }

    private:
        GameObject* m_object = nullptr; // non-owning
        std::function<GameObject* (SceneContext&)> m_respawn;
    };

    /**
     * @brief 複数のGameObjectをshared_ptrで管理するコレクションクラス
     * - SceneBaseでの汎用的なGameObject管理用
     * - スレッドセーフ操作は現時点では非対応（必要に応じて追加可能）
     */
    class ObjectSlotCollection final {
    public:
        ObjectSlotCollection() = default;

        /// @brief GameObjectを追加
        /// @param object 追加するGameObject（shared_ptrで所有権を共有）
        void Add(const std::shared_ptr<GameObject>& object) {
            m_objects.emplace_back(object);
        }

        /// @brief GameObjectを削除
        /// @param object 削除するGameObject
        /// @note O(n)の計算量。頻繁な追加・削除が必要な場合は別のデータ構造を検討してください
        void Remove(const std::shared_ptr<GameObject>& object) {
            m_objects.erase(std::remove(m_objects.begin(), m_objects.end(), object), m_objects.end());
        }

        /// @brief 全オブジェクトに対して処理を実行
        /// @tparam Func 処理関数型
        /// @param func 各オブジェクトに対して実行する関数
        template<typename Func>
        void ForEach(Func&& func) const {
            for (const auto& obj : m_objects) {
                func(obj);
            }
        }

        /// @brief 全オブジェクトをクリア
        void Clear() {
            m_objects.clear();
        }

        /// @brief オブジェクト数を取得
        /// @return オブジェクト数
        [[nodiscard]] std::size_t Count() const {
            return m_objects.size();
        }

        /// @brief コレクションが空かどうか確認
        /// @return 空ならtrue
        [[nodiscard]] bool IsEmpty() const {
            return m_objects.empty();
        }

        /// @brief 内部コンテナへの参照を取得（互換性維持用）
        /// @return オブジェクトコンテナへの参照
        [[nodiscard]] std::vector<std::shared_ptr<GameObject>>& GetContainer() {
            return m_objects;
        }

        /// @brief 内部コンテナへのconst参照を取得（互換性維持用）
        /// @return オブジェクトコンテナへのconst参照
        [[nodiscard]] const std::vector<std::shared_ptr<GameObject>>& GetContainer() const {
            return m_objects;
        }

    private:
        std::vector<std::shared_ptr<GameObject>> m_objects;
    };

} // namespace Engine
