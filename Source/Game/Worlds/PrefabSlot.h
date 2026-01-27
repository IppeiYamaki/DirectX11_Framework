#pragma once

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "Game/Worlds/WorldContext.h"
#include "Engine/Scene/World.h"

namespace Engine { class Entity; }

namespace Game {

    /**
     * @brief Prefabインスタンス管理用の枠
     * - Worldで「あとで削除/再生成/追従」したい対象をまとめて扱える
     * - Spawn時の引数を保存し、Respawnで同引数で再生成できる
     *
     * 注意：
     * - Destroy() は World::DestroyEntity() を呼ぶ（即時削除）
     * - Component Update内など、World内からのDestroyしないこと（World Update内ならOK）
     */
    class PrefabSlot final {
    public:
        PrefabSlot() = default;

        Engine::Entity* Get() const { return m_entity; }
        bool IsAlive() const { return m_entity != nullptr; }

        template<class TPrefab, class... Args>
        Engine::Entity* Spawn(WorldContext& ctx, Args&&... args) {
            // 引数を値として保持（あとで同引数で再生成するため）
            auto argsTuple = std::make_shared<std::tuple<std::decay_t<Args>...>>(
                std::forward<Args>(args)...);

            m_respawn = [argsTuple](WorldContext& c) -> Engine::Entity* {
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

        void Destroy(WorldContext& ctx) {
            if (!m_entity) return;
            if (ctx.m_world) {
                ctx.m_world->DestroyEntity(m_entity);
            }
            m_entity = nullptr;
        }

        Engine::Entity* Respawn(WorldContext& ctx) {
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
        std::function<Engine::Entity* (WorldContext&)> m_respawn;
    };

} // namespace Game
