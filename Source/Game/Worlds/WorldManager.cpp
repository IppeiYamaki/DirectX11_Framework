#include "WorldManager.h"

namespace Game {

    void WorldManager::Initialize(WorldContext ctx, std::unique_ptr<IWorld> firstWorld) {
        m_ctx = ctx;
        m_current = std::move(firstWorld);
        m_next.reset();

        m_isInitialized = true;

        if (m_current) {
            m_current->OnEnter(m_ctx);
        }
    }

    void WorldManager::Finalize() {
        if (!m_isInitialized) return;

        if (m_current) {
            m_current->OnExit(m_ctx);
            m_current.reset();
        }
        m_next.reset();

        m_isInitialized = false;
    }

    void WorldManager::ChangeWorld(std::unique_ptr<IWorld> nextWorld) {
        m_next = std::move(nextWorld);
    }

    void WorldManager::ApplyPendingWorldIfNeeded() {
        if (!m_next) return;

        if (m_current) {
            m_current->OnExit(m_ctx);
        }

        m_current = std::move(m_next);

        if (m_current) {
            m_current->OnEnter(m_ctx);
        }
    }

    void WorldManager::Update(float deltaTime) {
        if (!m_isInitialized) return;

        // フレームの先頭で切替（安全）
        ApplyPendingWorldIfNeeded();

        if (m_current) {
            m_current->Update(m_ctx, deltaTime);
        }
    }

    void WorldManager::Draw() {
        if (!m_isInitialized) return;

        if (m_current) {
            m_current->Draw(m_ctx);
        }
    }

} // namespace Game
