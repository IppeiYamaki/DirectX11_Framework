#include "SceneManager.h"

namespace Engine {

    void SceneManager::Initialize(SceneContext ctx, std::unique_ptr<SceneBase> firstScene) {
        m_ctx = ctx;
        m_current = std::move(firstScene);
        m_next.reset();

        m_isInitialized = true;

        if (m_current) {
            m_current->Initialize(m_ctx);
        }
    }

    void SceneManager::Finalize() {
        if (!m_isInitialized) return;

        if (m_current) {
            m_current->Finalize(m_ctx);
            m_current.reset();
        }
        m_next.reset();

        m_isInitialized = false;
    }

    void SceneManager::ChangeScene(std::unique_ptr<SceneBase> nextScene) {
        m_next = std::move(nextScene);
    }

    void SceneManager::ApplyPendingSceneIfNeeded() {
        if (!m_next) return;

        if (m_current) {
            m_current->Finalize(m_ctx);
        }

        m_current = std::move(m_next);

        if (m_current) {
            m_current->Initialize(m_ctx);
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

    void SceneManager::Render() {
        if (!m_isInitialized) return;

        if (m_current) {
            m_current->Render(m_ctx);
        }
    }

} // namespace Engine
