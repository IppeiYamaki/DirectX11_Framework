#include "SceneManager.h"

namespace Game {

    void SceneManager::Initialize(SceneContext ctx, std::unique_ptr<IScene> firstScene) {
        m_ctx = ctx;
        m_current = std::move(firstScene);
        m_next.reset();

        m_isInitialized = true;

        if (m_current) {
            m_current->OnEnter(m_ctx);
        }
    }

    void SceneManager::Finalize() {
        if (!m_isInitialized) return;

        if (m_current) {
            m_current->OnExit(m_ctx);
            m_current.reset();
        }
        m_next.reset();

        m_isInitialized = false;
    }

    void SceneManager::ChangeScene(std::unique_ptr<IScene> nextScene) {
        m_next = std::move(nextScene);
    }

    void SceneManager::ApplyPendingSceneIfNeeded() {
        if (!m_next) return;

        if (m_current) {
            m_current->OnExit(m_ctx);
        }

        m_current = std::move(m_next);

        if (m_current) {
            m_current->OnEnter(m_ctx);
        }
    }

    void SceneManager::Update(float deltaTime) {
        if (!m_isInitialized) return;

        // フレームの先頭で切替（安全）
        ApplyPendingSceneIfNeeded();

        if (m_current) {
            m_current->Update(m_ctx, deltaTime);
        }
    }

    void SceneManager::Draw() {
        if (!m_isInitialized) return;

        if (m_current) {
            m_current->Draw(m_ctx);
        }
    }

} // namespace Game
