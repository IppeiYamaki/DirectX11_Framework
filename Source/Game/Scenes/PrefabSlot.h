#pragma once

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/Scene.h"

namespace Engine { class Entity; }

namespace Game {

    /**
     * @brief Prefabインスタンス管理用の枠
     * - Sceneで「あとで削除/再生成/追従」したい対象をまとめて扱える
     * - Spawn時の引数を保存し、Respawnで同引数で再生成できる
     *
     * 注意：
     * - Destroy() は Scene::DestroyEntity() を呼ぶ（即時削除）
     * - Component Update内など、Scene内からのDestroyしないこと（Scene Update内ならOK）
     */
    class PrefabSlot final {
    public:
        PrefabSlot() = default;

        Engine::Entity* Get() const { return m_entity; }
        bool IsAlive() const { return m_entity != nullptr; }

        template<class TPrefab, class... Args>
        Engine::Entity* Spawn(Engine::SceneContext& ctx, Args&&... args) {
            // 引数を値として保持（あとで同引数で再生成するため）
            auto argsTuple = std::make_shared<std::tuple<std::decay_t<Args>...>>(
                std::forward<Args>(args)...);

            m_respawn = [argsTuple](Engine::SceneContext& c) -> Engine::Entity* {
                return std::apply(
                    [&](auto&&... a) -> Engine::Entity* {
                        return c.Spawn<TPrefab>(a...);
                    },
                    *argsTuple);
                };

            // 既に生成してるなら破棄してから生成（転用をシンプルに）
            Destroy(ctx);
            m_entity = m_respawn(ctx);
            return m_entity;
        }

        void Destroy(Engine::SceneContext& ctx) {
            if (!m_entity) return;
            if (ctx.m_scene) {
                ctx.m_scene->DestroyEntity(m_entity);
            }
            m_entity = nullptr;
        }

        Engine::Entity* Respawn(Engine::SceneContext& ctx) {
            Destroy(ctx);
            if (!m_respawn) return nullptr;
            m_entity = m_respawn(ctx);
            return m_entity;
        }

        void Clear() {
            m_entity = nullptr;
            m_respawn = nullptr;
        }

    private:
        Engine::Entity* m_entity = nullptr; // non-owning
        std::function<Engine::Entity* (Engine::SceneContext&)> m_respawn;
    };

} // namespace Game
