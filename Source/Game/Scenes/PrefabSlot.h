#pragma once

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "Game/Scenes/SceneContext.h"
#include "Engine/Scene/World.h"

namespace Engine { class Entity; }

namespace Game {

    /**
     * @brief Prefabインスタンス管理用の枠
     * - Sceneが「あとで消す/再生成/追従」したい対象をまとめて扱える
     * - Spawn時の引数を保存し、Respawnで同条件で再生成できる
     *
     * 注意：
     * - Destroy() は World::DestroyEntity() を呼ぶ（即時削除）
     * - Component Update中など、Worldが走査中にDestroyしないこと（Scene Update中ならOK）
     */
    class PrefabSlot final {
    public:
        PrefabSlot() = default;

        Engine::Entity* Get() const { return m_entity; }
        bool IsAlive() const { return m_entity != nullptr; }

        template<class TPrefab, class... Args>
        Engine::Entity* Spawn(SceneContext& ctx, Args&&... args) {
            // 引数を値として保持（あとで同条件で再生成するため）
            auto argsTuple = std::make_shared<std::tuple<std::decay_t<Args>...>>(
                std::forward<Args>(args)...);

            m_respawn = [argsTuple](SceneContext& c) -> Engine::Entity* {
                return std::apply(
                    [&](auto&&... a) -> Engine::Entity* {
                        return c.Spawn<TPrefab>(a...);
                    },
                    *argsTuple);
                };

            // すでに生きているなら一旦破棄してから生成（運用をシンプルに）
            Destroy(ctx);
            m_entity = m_respawn(ctx);
            return m_entity;
        }

        void Destroy(SceneContext& ctx) {
            if (!m_entity) return;
            if (ctx.m_world) {
                ctx.m_world->DestroyEntity(m_entity);
            }
            m_entity = nullptr;
        }

        Engine::Entity* Respawn(SceneContext& ctx) {
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
        std::function<Engine::Entity* (SceneContext&)> m_respawn;
    };

} // namespace Game
