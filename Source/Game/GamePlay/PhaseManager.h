/// @file   PhaseManager.h
/// @brief  ゲームフェーズ管理クラス
#pragma once

#include <functional>
#include <string>

namespace Game {

    /// @brief ゲームフェーズの種類
    enum class GamePhase {
        SetupPhase,         ///< 準備フェーズ（部屋・トラップの配置）
        OperationPhase,     ///< 運用フェーズ（敵の移動・行動）
        RewardPhase,        ///< 報酬選択フェーズ
        GameOver            ///< ゲームオーバー
    };

    /// @brief ゲームフェーズ管理クラス
    /// @note  SetupPhaseとOperationPhaseの状態遷移を管理
    class PhaseManager {
    public:
        /// @brief コンストラクタ
        PhaseManager();

        /// @brief デストラクタ
        ~PhaseManager() = default;

        // コピー禁止
        PhaseManager(const PhaseManager&) = delete;
        PhaseManager& operator=(const PhaseManager&) = delete;

        //============================================================
        // フェーズ管理
        //============================================================

        /// @brief 現在のフェーズを取得
        /// @return 現在のフェーズ
        [[nodiscard]] GamePhase GetCurrentPhase() const;

        /// @brief フェーズ名を取得
        /// @return フェーズ名の文字列
        [[nodiscard]] std::string GetCurrentPhaseName() const;

        /// @brief 次のフェーズに遷移
        void TransitionToNextPhase();

        /// @brief 指定したフェーズに遷移
        /// @param phase 遷移先のフェーズ
        void TransitionToPhase(GamePhase phase);

        /// @brief ゲームを開始（SetupPhaseから）
        void StartGame();

        /// @brief ゲームをリセット
        void ResetGame();

        //============================================================
        // ターン・日数管理
        //============================================================

        /// @brief 現在のターン数を取得
        /// @return 現在のターン数
        [[nodiscard]] int GetCurrentTurn() const;

        /// @brief 現在の日数を取得
        /// @return 現在の日数
        [[nodiscard]] int GetCurrentDay() const;

        /// @brief ターンを進める
        void AdvanceTurn();

        /// @brief 日数を進める
        void AdvanceDay();

        //============================================================
        // 行動ポイント管理
        //============================================================

        /// @brief 現在の行動ポイントを取得
        /// @return 現在の行動ポイント
        [[nodiscard]] int GetActionPoints() const;

        /// @brief 最大行動ポイントを取得
        /// @return 最大行動ポイント
        [[nodiscard]] int GetMaxActionPoints() const;

        /// @brief 行動ポイントを消費
        /// @param points 消費するポイント数
        /// @return 消費成功ならtrue（0ポイント消費は常に成功、負の値は常に失敗）
        bool UseActionPoints(int points);

        /// @brief 行動ポイントを回復
        /// @param points 回復するポイント数（負の値は無視される）
        /// @note  最大値を超える場合は最大値にクランプされる
        void RestoreActionPoints(int points);

        /// @brief 行動ポイントを最大値まで回復
        void RefillActionPoints();

        /// @brief 最大行動ポイントを設定
        /// @param maxPoints 最大行動ポイント
        /// @note  現在のポイントが新しい最大値を超える場合はクランプされる
        ///        最大値を増やしても現在のポイントは自動的に増加しない
        void SetMaxActionPoints(int maxPoints);

        //============================================================
        // 勝敗条件
        //============================================================

        /// @brief 敗北条件：資料が奪われた回数を取得
        /// @return 資料が奪われた回数
        [[nodiscard]] int GetDocumentsStolen() const;

        /// @brief 敗北条件：許容される最大盗難回数を取得
        /// @return 許容される最大盗難回数
        [[nodiscard]] int GetMaxDocumentsAllowed() const;

        /// @brief 資料が奪われた
        void OnDocumentStolen();

        /// @brief 許容される最大盗難回数を設定
        /// @param maxDocuments 許容される最大盗難回数
        void SetMaxDocumentsAllowed(int maxDocuments);

        /// @brief ゲームオーバーか確認
        /// @return ゲームオーバーならtrue
        [[nodiscard]] bool IsGameOver() const;

        /// @brief 勝利条件が満たされたか確認
        /// @param enemiesRemaining 残りの敵の数
        /// @return 勝利条件が満たされていればtrue
        [[nodiscard]] bool IsVictory(int enemiesRemaining) const;

        //============================================================
        // コールバック設定
        //============================================================

        /// @brief フェーズ開始時のコールバックを設定
        /// @param callback フェーズ開始時に呼び出される関数
        void SetOnPhaseStart(std::function<void(GamePhase)> callback);

        /// @brief フェーズ終了時のコールバックを設定
        /// @param callback フェーズ終了時に呼び出される関数
        void SetOnPhaseEnd(std::function<void(GamePhase)> callback);

        /// @brief ゲームオーバー時のコールバックを設定
        /// @param callback ゲームオーバー時に呼び出される関数
        void SetOnGameOver(std::function<void()> callback);

        /// @brief 勝利時のコールバックを設定
        /// @param callback 勝利時に呼び出される関数
        void SetOnVictory(std::function<void()> callback);

        //============================================================
        // 更新処理
        //============================================================

        /// @brief フレーム更新処理
        /// @param deltaTime フレーム経過時間
        virtual void Update(float deltaTime);

    protected:
        /// @brief SetupPhase開始時の処理
        virtual void OnSetupPhaseStart();

        /// @brief SetupPhase終了時の処理
        virtual void OnSetupPhaseEnd();

        /// @brief OperationPhase開始時の処理
        virtual void OnOperationPhaseStart();

        /// @brief OperationPhase終了時の処理
        virtual void OnOperationPhaseEnd();

        /// @brief RewardPhase開始時の処理
        virtual void OnRewardPhaseStart();

        /// @brief RewardPhase終了時の処理
        virtual void OnRewardPhaseEnd();

        /// @brief GameOver時の処理
        virtual void OnGameOverStart();

    private:
        GamePhase m_currentPhase = GamePhase::SetupPhase;   ///< 現在のフェーズ

        int m_currentTurn = 0;              ///< 現在のターン数
        int m_currentDay = 1;               ///< 現在の日数

        int m_actionPoints = 5;             ///< 現在の行動ポイント
        int m_maxActionPoints = 5;          ///< 最大行動ポイント

        int m_documentsStolen = 0;          ///< 盗まれた資料の数
        int m_maxDocumentsAllowed = 3;      ///< 許容される最大盗難回数

        // コールバック
        std::function<void(GamePhase)> m_onPhaseStart;  ///< フェーズ開始コールバック
        std::function<void(GamePhase)> m_onPhaseEnd;    ///< フェーズ終了コールバック
        std::function<void()> m_onGameOver;             ///< ゲームオーバーコールバック
        std::function<void()> m_onVictory;              ///< 勝利コールバック
    };

} // namespace Game
