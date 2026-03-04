/// @file   MazeGameManager.h
/// @brief  迷路ゲームのマネージャー（タイマー、スコア、クリア判定）
#pragma once

#include <vector>
#include <string>

#include "Engine/Math/Vector3.h"

namespace Engine {
    class TextElement;
    class Canvas;
}

namespace Game {

    class CollectibleBallComponent;
    class CollectibleParticleComponent;

    /// @brief 迷路ゲーム状態
    enum class MazeGameState {
        Playing,    ///< プレイ中
        Cleared     ///< クリア
    };

    /// @brief 迷路ゲームのマネージャー
    class MazeGameManager {
    public:
        MazeGameManager() = default;
        ~MazeGameManager() = default;

        //============================================================
        // 初期化
        //============================================================

        /// @brief 初期化
        /// @param canvas UI用Canvas
        /// @param totalBalls 全球体数
        void Initialize(Engine::Canvas* canvas, int totalBalls);

        /// @brief リセット
        void Reset(int totalBalls);

        //============================================================
        // 更新
        //============================================================

        /// @brief 毎フレーム更新
        /// @param deltaTime フレーム経過時間
        void Update(float deltaTime);

        //============================================================
        // 球体取得
        //============================================================

        /// @brief 球体取得時に呼ばれる
        void OnBallCollected();

        /// @brief 球体コンポーネントを登録
        void RegisterBall(CollectibleBallComponent* ball);

        /// @brief パーティクルコンポーネントを登録
        void RegisterParticle(CollectibleParticleComponent* particle);

        /// @brief プレイヤー位置を更新（全収集物に伝播）
        void UpdatePlayerPosition(const Engine::Vector3& pos);

        //============================================================
        // 状態取得
        //============================================================

        /// @brief ゲーム状態を取得
        [[nodiscard]] MazeGameState GetState() const { return m_state; }

        /// @brief クリア済みかチェック
        [[nodiscard]] bool IsCleared() const { return m_state == MazeGameState::Cleared; }

        /// @brief 取得数を取得
        [[nodiscard]] int GetCollectedCount() const { return m_collectedCount; }

        /// @brief 全球体数を取得
        [[nodiscard]] int GetTotalBalls() const { return m_totalBalls; }

        /// @brief 経過時間を取得
        [[nodiscard]] float GetElapsedTime() const { return m_elapsedTime; }

        //============================================================
        // UI
        //============================================================

        /// @brief タイマー表示フォーマット
        [[nodiscard]] std::wstring FormatTime(float seconds) const;

    private:
        /// @brief UIを更新
        void UpdateUI();

        /// @brief クリア処理
        void OnGameCleared();

    private:
        MazeGameState m_state = MazeGameState::Playing;

        int m_collectedCount = 0;
        int m_totalBalls = 0;
        float m_elapsedTime = 0.0f;

        // 登録された球体
        std::vector<CollectibleBallComponent*> m_balls;

        // 登録されたパーティクル
        std::vector<CollectibleParticleComponent*> m_particles;

        // UI要素
        Engine::Canvas* m_canvas = nullptr;
        Engine::TextElement* m_timerText = nullptr;
        Engine::TextElement* m_countText = nullptr;
        Engine::TextElement* m_clearText = nullptr;
        Engine::TextElement* m_restartText = nullptr;
    };

} // namespace Game
