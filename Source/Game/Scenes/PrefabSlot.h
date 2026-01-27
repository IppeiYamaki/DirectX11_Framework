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
     * @brief Prefabインスタンス管理用の枠
     * - Sceneで「あとで削除/再生成/追従」したい対象をまとめて扱える
     * - Spawn時の引数を保存し、Respawnで同引数で再生成できる
     *
     * 注意：
     * - Destroy() は Scene::DestroyObject() を呼ぶ（即時削除）
     * - Component Update内など、Scene内からのDestroyしないこと（Scene Update内ならOK）
     */
    class PrefabSlot final {
    public:
        PrefabSlot() = default;

        Engine::GameObject* Get() const { return m_object; }
        bool IsAlive() const { return m_object != nullptr; }

        template<class TPrefab, class... Args>
        Engine::GameObject* Spawn(Game::SceneContext& ctx, Args&&... args) {
            // 引数を値として保持（あとで同引数で再生成するため）
            auto argsTuple = std::make_shared<std::tuple<std::decay_t<Args>...>>(
                std::forward<Args>(args)...);

            m_respawn = [argsTuple](Game::SceneContext& c) -> Engine::GameObject* {
                return std::apply(
                    [&](auto&&... a) -> Engine::GameObject* {
                        return c.Spawn<TPrefab>(a...);
                    },
                    *argsTuple);
                };

            // 既に生成してるなら破棄してから生成（転用をシンプルに）
            Destroy(ctx);
            m_object = m_respawn(ctx);
            return m_object;
        }

        void Destroy(Game::SceneContext& ctx) {
            if (!m_object) return;
            if (ctx.m_scene) {
                ctx.m_scene->DestroyObject(m_object);
            }
            m_object = nullptr;
        }

        Engine::GameObject* Respawn(Game::SceneContext& ctx) {
            Destroy(ctx);
            if (!m_respawn) return nullptr;
            m_object = m_respawn(ctx);
            return m_object;
        }

        void Clear() {
            m_object = nullptr;
            m_respawn = nullptr;
        }

    private:
        Engine::GameObject* m_object = nullptr; // non-owning
        std::function<Engine::GameObject* (Game::SceneContext&)> m_respawn;
    };

} // namespace Game
