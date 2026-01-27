#pragma once

#include <memory>

#include "Game/Worlds/IWorld.h"
#include "Game/Worlds/WorldContext.h"

namespace Game {

    /**
     * @brief World（State）を管理する
     *
     * - ChangeWorld は "予約" される
     * - 実際の切替は Update の先頭で行う（フレーム内に安全に切替）
     */
    class WorldManager final {
    public:
        WorldManager() = default;
        ~WorldManager() = default;

        WorldManager(const WorldManager&) = delete;
        WorldManager& operator=(const WorldManager&) = delete;

        void Initialize(WorldContext ctx, std::unique_ptr<IWorld> firstWorld);
        void Finalize();

        void Update(float deltaTime);
        void Draw();

        void ChangeWorld(std::unique_ptr<IWorld> nextWorld);

        template<class T, class... Args>
        void ChangeWorldTo(Args&&... args) {
            ChangeWorld(std::make_unique<T>(std::forward<Args>(args)...));
        }

    private:
        void ApplyPendingWorldIfNeeded();

    private:
        WorldContext m_ctx{};

        std::unique_ptr<IWorld> m_current;
        std::unique_ptr<IWorld> m_next;

        bool m_isInitialized = false;
    };

} // namespace Game
