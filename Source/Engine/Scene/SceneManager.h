#pragma once

#include <memory>

#include "Engine/Scene/SceneBase.h"
#include "Engine/Scene/SceneContext.h"

namespace Engine {

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

		/// @brief  初期化
		/// @param  ctx Scene利用コンテキスト情報
        /// @param  firstScene 最初のシーン
        /// @return 初期化成功ならtrue
        bool Initialize(SceneContext ctx, std::unique_ptr<SceneBase> firstScene);
		/// @brief 終了処理
        void Finalize();

		/// @brief 毎フレーム更新
		/// @param deltaTime フレーム経過時間
        void Update(float deltaTime);
		/// @brief 毎フレーム描画
        void Render();

		/// @brief  シーン切替予約
		/// @param  nextScene 切替先シーン
        void ChangeScene(std::unique_ptr<SceneBase> nextScene);

        template<class T, class... Args>
        void ChangeSceneTo(Args&&... args) {
            ChangeScene(std::make_unique<T>(std::forward<Args>(args)...));
        }

		const SceneBase* GetCurrentScene() const { return m_current.get(); }

    private:

		/// @brief  予約されたシーンがあれば切替を実行
		/// @note   Updateの先頭で呼び出すこと
        void ApplyPendingSceneIfNeeded();

    private:
        SceneContext m_ctx{};

        std::unique_ptr<SceneBase> m_current;
        std::unique_ptr<SceneBase> m_next;

        bool m_isInitialized = false;
    };

} // namespace Engine
