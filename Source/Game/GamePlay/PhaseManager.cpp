/// @file   PhaseManager.cpp
/// @brief  ゲームフェーズ管理クラス実装
#include "Game/Gameplay/PhaseManager.h"
#include "Engine/Core/Logger.h"

namespace Game {

    PhaseManager::PhaseManager()
        : m_currentPhase(GamePhase::SetupPhase)
        , m_currentTurn(0)
        , m_currentDay(1)
        , m_actionPoints(5)
        , m_maxActionPoints(5)
        , m_documentsStolen(0)
        , m_maxDocumentsAllowed(3)
    {
    }

    //============================================================
    // フェーズ管理
    //============================================================

    GamePhase PhaseManager::GetCurrentPhase() const {
        return m_currentPhase;
    }

    std::string PhaseManager::GetCurrentPhaseName() const {
        switch (m_currentPhase) {
        case GamePhase::SetupPhase:
            return "SetupPhase";
        case GamePhase::OperationPhase:
            return "OperationPhase";
        case GamePhase::RewardPhase:
            return "RewardPhase";
        case GamePhase::GameOver:
            return "GameOver";
        default:
            return "Unknown";
        }
    }

    void PhaseManager::TransitionToNextPhase() {
        // 現在のフェーズの終了処理
        if (m_onPhaseEnd) {
            m_onPhaseEnd(m_currentPhase);
        }

        switch (m_currentPhase) {
        case GamePhase::SetupPhase:
            OnSetupPhaseEnd();
            m_currentPhase = GamePhase::OperationPhase;
            OnOperationPhaseStart();
            break;

        case GamePhase::OperationPhase:
            OnOperationPhaseEnd();
            // 敵が全滅していなければ報酬フェーズに遷移
            m_currentPhase = GamePhase::RewardPhase;
            OnRewardPhaseStart();
            break;

        case GamePhase::RewardPhase:
            OnRewardPhaseEnd();
            AdvanceDay();
            m_currentPhase = GamePhase::SetupPhase;
            OnSetupPhaseStart();
            break;

        case GamePhase::GameOver:
            // ゲームオーバーからは遷移しない
            break;
        }

        // 新しいフェーズの開始処理
        if (m_onPhaseStart) {
            m_onPhaseStart(m_currentPhase);
        }

        Engine::Logger::Info("Phase transitioned to: " + GetCurrentPhaseName());
    }

    void PhaseManager::TransitionToPhase(GamePhase phase) {
        // 現在のフェーズの終了処理
        if (m_onPhaseEnd) {
            m_onPhaseEnd(m_currentPhase);
        }

        switch (m_currentPhase) {
        case GamePhase::SetupPhase:
            OnSetupPhaseEnd();
            break;
        case GamePhase::OperationPhase:
            OnOperationPhaseEnd();
            break;
        case GamePhase::RewardPhase:
            OnRewardPhaseEnd();
            break;
        case GamePhase::GameOver:
            break;
        }

        m_currentPhase = phase;

        // 新しいフェーズの開始処理
        switch (m_currentPhase) {
        case GamePhase::SetupPhase:
            OnSetupPhaseStart();
            break;
        case GamePhase::OperationPhase:
            OnOperationPhaseStart();
            break;
        case GamePhase::RewardPhase:
            OnRewardPhaseStart();
            break;
        case GamePhase::GameOver:
            OnGameOverStart();
            break;
        }

        if (m_onPhaseStart) {
            m_onPhaseStart(m_currentPhase);
        }

        Engine::Logger::Info("Phase set to: " + GetCurrentPhaseName());
    }

    void PhaseManager::StartGame() {
        m_currentPhase = GamePhase::SetupPhase;
        m_currentTurn = 0;
        m_currentDay = 1;
        m_documentsStolen = 0;
        RefillActionPoints();
        OnSetupPhaseStart();

        if (m_onPhaseStart) {
            m_onPhaseStart(m_currentPhase);
        }

        Engine::Logger::Info("Game started - Day 1, SetupPhase");
    }

    void PhaseManager::ResetGame() {
        m_currentPhase = GamePhase::SetupPhase;
        m_currentTurn = 0;
        m_currentDay = 1;
        m_documentsStolen = 0;
        RefillActionPoints();

        Engine::Logger::Info("Game reset");
    }

    //============================================================
    // ターン・日数管理
    //============================================================

    int PhaseManager::GetCurrentTurn() const {
        return m_currentTurn;
    }

    int PhaseManager::GetCurrentDay() const {
        return m_currentDay;
    }

    void PhaseManager::AdvanceTurn() {
        ++m_currentTurn;
        Engine::Logger::Trace("Turn advanced to: " + std::to_string(m_currentTurn));
    }

    void PhaseManager::AdvanceDay() {
        ++m_currentDay;
        m_currentTurn = 0;
        RefillActionPoints();
        Engine::Logger::Info("Day advanced to: " + std::to_string(m_currentDay));
    }

    //============================================================
    // 行動ポイント管理
    //============================================================

    int PhaseManager::GetActionPoints() const {
        return m_actionPoints;
    }

    int PhaseManager::GetMaxActionPoints() const {
        return m_maxActionPoints;
    }

    bool PhaseManager::UseActionPoints(int points) {
        if (points < 0) {
            // 負の値は無効
            return false;
        }
        if (points == 0) {
            // 0ポイントの消費は成功とみなす
            return true;
        }
        if (m_actionPoints < points) {
            return false;
        }
        m_actionPoints -= points;
        return true;
    }

    void PhaseManager::RestoreActionPoints(int points) {
        if (points < 0) {
            // 負の値は無視（減少させたい場合はUseActionPointsを使用）
            return;
        }
        m_actionPoints += points;
        if (m_actionPoints > m_maxActionPoints) {
            m_actionPoints = m_maxActionPoints;
        }
    }

    void PhaseManager::RefillActionPoints() {
        m_actionPoints = m_maxActionPoints;
    }

    void PhaseManager::SetMaxActionPoints(int maxPoints) {
        m_maxActionPoints = maxPoints;
        if (m_actionPoints > m_maxActionPoints) {
            m_actionPoints = m_maxActionPoints;
        }
    }

    //============================================================
    // 勝敗条件
    //============================================================

    int PhaseManager::GetDocumentsStolen() const {
        return m_documentsStolen;
    }

    int PhaseManager::GetMaxDocumentsAllowed() const {
        return m_maxDocumentsAllowed;
    }

    void PhaseManager::OnDocumentStolen() {
        ++m_documentsStolen;
        Engine::Logger::Warn("Document stolen! Total: " + std::to_string(m_documentsStolen) + 
                            "/" + std::to_string(m_maxDocumentsAllowed));

        if (IsGameOver()) {
            TransitionToPhase(GamePhase::GameOver);
            if (m_onGameOver) {
                m_onGameOver();
            }
        }
    }

    void PhaseManager::SetMaxDocumentsAllowed(int maxDocuments) {
        m_maxDocumentsAllowed = maxDocuments;
    }

    bool PhaseManager::IsGameOver() const {
        return m_documentsStolen >= m_maxDocumentsAllowed ||
               m_currentPhase == GamePhase::GameOver;
    }

    bool PhaseManager::IsVictory(int enemiesRemaining) const {
        return enemiesRemaining <= 0 && !IsGameOver();
    }

    //============================================================
    // コールバック設定
    //============================================================

    void PhaseManager::SetOnPhaseStart(std::function<void(GamePhase)> callback) {
        m_onPhaseStart = std::move(callback);
    }

    void PhaseManager::SetOnPhaseEnd(std::function<void(GamePhase)> callback) {
        m_onPhaseEnd = std::move(callback);
    }

    void PhaseManager::SetOnGameOver(std::function<void()> callback) {
        m_onGameOver = std::move(callback);
    }

    void PhaseManager::SetOnVictory(std::function<void()> callback) {
        m_onVictory = std::move(callback);
    }

    //============================================================
    // 更新処理
    //============================================================

    void PhaseManager::Update(float /*deltaTime*/) {
        // フェーズごとの更新処理
        // 派生クラスでオーバーライドして具体的な処理を実装
    }

    //============================================================
    // Protected
    //============================================================

    void PhaseManager::OnSetupPhaseStart() {
        Engine::Logger::Info("SetupPhase started - Day " + std::to_string(m_currentDay));
        RefillActionPoints();
    }

    void PhaseManager::OnSetupPhaseEnd() {
        Engine::Logger::Info("SetupPhase ended");
    }

    void PhaseManager::OnOperationPhaseStart() {
        Engine::Logger::Info("OperationPhase started");
        m_currentTurn = 0;
    }

    void PhaseManager::OnOperationPhaseEnd() {
        Engine::Logger::Info("OperationPhase ended");
    }

    void PhaseManager::OnRewardPhaseStart() {
        Engine::Logger::Info("RewardPhase started");
    }

    void PhaseManager::OnRewardPhaseEnd() {
        Engine::Logger::Info("RewardPhase ended");
    }

    void PhaseManager::OnGameOverStart() {
        Engine::Logger::Error("Game Over!");
    }

} // namespace Game
