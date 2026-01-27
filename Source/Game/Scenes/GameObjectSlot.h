/// @file   GameObjectSlot.h
/// @brief  GameObjectインスタンス管理用の枠
#pragma once

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/Scene.h"

namespace Engine { class GameObject; }

namespace Game {

    /**
     * @brief GameObjectインスタンス管理用の枠
     * - Sceneで「あとで削除/再生成/追従」したい対象をまとめて扱える
     * - Spawn時の引数を保存し、Respawnで同引数で再生成できる
     *
     * 注意：
     * - Destroy() は Scene::DestroyObject() を呼ぶ（即時削除）
     * - Component Update内など、Scene内からのDestroyしないこと（Scene Update内ならOK）
     */
    class GameObjectSlot final {
    public:
        GameObjectSlot() = default;

        /// @brief 管理しているGameObjectを取得
        /// @return GameObjectへのポインタ
        Engine::GameObject* Get() const { return m_object; }

        /// @brief GameObjectが生存しているか確認
        /// @return 生存していればtrue
        bool IsAlive() const { return m_object != nullptr; }

        /// @brief GameObjectPrefabを使って生成
        /// @tparam TPrefab PrefabクラスSpawnObjectを持つ
        /// @param ctx SceneContext
        /// @param args Prefab::SpawnDescのコンストラクタ引数
        /// @return 生成されたGameObject
        template<class TPrefab, class... Args>
        Engine::GameObject* Spawn(Engine::SceneContext& ctx, Args&&... args) {
            // 引数を値として保持（あとで同引数で再生成するため）
            auto argsTuple = std::make_shared<std::tuple<std::decay_t<Args>...>>(
                std::forward<Args>(args)...);

            m_respawn = [argsTuple](Engine::SceneContext& c) -> Engine::GameObject* {
                return std::apply(
                    [&](auto&&... a) -> Engine::GameObject* {
                        return c.SpawnObject<TPrefab>(a...);
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
        void Destroy(Engine::SceneContext& ctx) {
            if (!m_object) return;
            if (ctx.m_scene) {
                ctx.m_scene->DestroyObject(m_object);
            }
            m_object = nullptr;
        }

        /// @brief 同じ引数で再生成
        /// @param ctx SceneContext
        /// @return 再生成されたGameObject
        Engine::GameObject* Respawn(Engine::SceneContext& ctx) {
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
        Engine::GameObject* m_object = nullptr; // non-owning
        std::function<Engine::GameObject* (Engine::SceneContext&)> m_respawn;
    };

} // namespace Game
