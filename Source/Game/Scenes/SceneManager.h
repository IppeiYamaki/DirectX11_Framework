#pragma once

#include <memory>

#include "Game/Scenes/IScene.h"
#include "Game/Scenes/SceneContext.h"

namespace Game {

    /**
     * @brief Scene（State）を管理する
     *
     * - ChangeScene は “予約” だけ
     * - 実際の切替は Update の先頭で行う（フレーム中に安全に切替）
     */
    class SceneManager final {
    public:
        SceneManager() = default;
        ~SceneManager() = default;

        SceneManager(const SceneManager&) = delete;
        SceneManager& operator=(const SceneManager&) = delete;

        void Initialize(SceneContext ctx, std::unique_ptr<IScene> firstScene);
        void Finalize();

        void Update(float deltaTime);
        void Draw();

        void ChangeScene(std::unique_ptr<IScene> nextScene);

        template<class T, class... Args>
        void ChangeSceneTo(Args&&... args) {
            ChangeScene(std::make_unique<T>(std::forward<Args>(args)...));
        }

    private:
        void ApplyPendingSceneIfNeeded();

    private:
        SceneContext m_ctx{};

        std::unique_ptr<IScene> m_current;
        std::unique_ptr<IScene> m_next;

        bool m_isInitialized = false;
    };

} // namespace Game
