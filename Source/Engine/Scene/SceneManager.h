#pragma once

#include <memory>

#include "Engine/Scene/SceneBase.h"
#include "Engine/Scene/SceneContext.h"

namespace Engine {

    using SceneContext = Game::SceneContext;

    /**
     * @brief Scene（State）を管理する
     *
     * - ChangeScene は "予約" される
     * - 実際の切替は Update の先頭で行う（フレーム内に安全に切替）
     */
    class SceneManager final {
    public:
        SceneManager() = default;
        ~SceneManager() = default;

        SceneManager(const SceneManager&) = delete;
        SceneManager& operator=(const SceneManager&) = delete;

        void Initialize(SceneContext ctx, std::unique_ptr<SceneBase> firstScene);
        void Finalize();

        void Update(float deltaTime);
        void Render();

        void ChangeScene(std::unique_ptr<SceneBase> nextScene);

        template<class T, class... Args>
        void ChangeSceneTo(Args&&... args) {
            ChangeScene(std::make_unique<T>(std::forward<Args>(args)...));
        }

    private:
        void ApplyPendingSceneIfNeeded();

    private:
        SceneContext m_ctx{};

        std::unique_ptr<SceneBase> m_current;
        std::unique_ptr<SceneBase> m_next;

        bool m_isInitialized = false;
    };

} // namespace Engine
