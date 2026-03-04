/// @file   MazeGameManager.cpp
/// @brief  迷路ゲームのマネージャー実装
#include "MazeGameManager.h"

#include "Engine/UI/Canvas.h"
#include "Engine/UI/TextElement.h"
#include "Engine/Math/Vector2.h"
#include "Game/Gameplay/Maze/CollectibleBallComponent.h"
#include "Game/Gameplay/Maze/CollectibleParticleComponent.h"

#include <sstream>
#include <iomanip>

namespace Game {

    void MazeGameManager::Initialize(Engine::Canvas* canvas, int totalBalls) {
        m_canvas = canvas;
        m_totalBalls = totalBalls;
        m_collectedCount = 0;
        m_elapsedTime = 0.0f;
        m_state = MazeGameState::Playing;
        m_balls.clear();
        m_particles.clear();

        if (!m_canvas) {
            return;
        }

        // タイマーテキスト
        m_timerText = m_canvas->CreateElement<Engine::TextElement>();
        if (m_timerText) {
            m_timerText->SetPosition(Engine::Vector2(20.0f, 20.0f));
            m_timerText->SetFontSize(32.0f);
            m_timerText->SetColor(Engine::Color(1.0f, 1.0f, 1.0f, 1.0f));
            m_timerText->SetText(L"Time: 00:00.000");
        }

        // カウントテキスト
        m_countText = m_canvas->CreateElement<Engine::TextElement>();
        if (m_countText) {
            m_countText->SetPosition(Engine::Vector2(20.0f, 60.0f));
            m_countText->SetFontSize(28.0f);
            m_countText->SetColor(Engine::Color(1.0f, 0.9f, 0.3f, 1.0f));
            m_countText->SetText(L"Collected: 0 / 0");
        }

        // クリアテキスト（初期は非表示）
        m_clearText = m_canvas->CreateElement<Engine::TextElement>();
        if (m_clearText) {
            m_clearText->SetPosition(Engine::Vector2(400.0f, 300.0f));
            m_clearText->SetFontSize(64.0f);
            m_clearText->SetColor(Engine::Color(0.3f, 1.0f, 0.3f, 1.0f));
            m_clearText->SetText(L"");
            m_clearText->SetHorizontalAlignment(Engine::TextElement::HorizontalAlignment::Center);
        }

        // リスタートテキスト（初期は非表示）
        m_restartText = m_canvas->CreateElement<Engine::TextElement>();
        if (m_restartText) {
            m_restartText->SetPosition(Engine::Vector2(400.0f, 380.0f));
            m_restartText->SetFontSize(24.0f);
            m_restartText->SetColor(Engine::Color(1.0f, 1.0f, 1.0f, 1.0f));
            m_restartText->SetText(L"");
            m_restartText->SetHorizontalAlignment(Engine::TextElement::HorizontalAlignment::Center);
        }

        UpdateUI();
    }

    void MazeGameManager::Reset(int totalBalls) {
        m_totalBalls = totalBalls;
        m_collectedCount = 0;
        m_elapsedTime = 0.0f;
        m_state = MazeGameState::Playing;
        m_balls.clear();
        m_particles.clear();

        if (m_clearText) {
            m_clearText->SetText(L"");
        }
        if (m_restartText) {
            m_restartText->SetText(L"");
        }

        UpdateUI();
    }

    void MazeGameManager::Update(float deltaTime) {
        if (m_state == MazeGameState::Playing) {
            m_elapsedTime += deltaTime;
            UpdateUI();
        }
    }

    void MazeGameManager::OnBallCollected() {
        if (m_state != MazeGameState::Playing) {
            return;
        }

        ++m_collectedCount;
        UpdateUI();

        // 全取得チェック
        if (m_collectedCount >= m_totalBalls) {
            OnGameCleared();
        }
    }

    void MazeGameManager::RegisterBall(CollectibleBallComponent* ball) {
        if (ball) {
            m_balls.push_back(ball);
        }
    }

    void MazeGameManager::RegisterParticle(CollectibleParticleComponent* particle) {
        if (particle) {
            m_particles.push_back(particle);
        }
    }

    void MazeGameManager::UpdatePlayerPosition(const Engine::Vector3& pos) {
        // 球体コンポーネントへの伝播
        for (auto* ball : m_balls) {
            if (ball && !ball->IsCollected()) {
                ball->SetPlayerPosition(pos);
            }
        }
        // パーティクルコンポーネントへの伝播
        for (auto* particle : m_particles) {
            if (particle && !particle->IsCollected()) {
                particle->SetPlayerPosition(pos);
            }
        }
    }

    std::wstring MazeGameManager::FormatTime(float seconds) const {
        int totalMs = static_cast<int>(seconds * 1000.0f);
        int minutes = totalMs / 60000;
        int secs = (totalMs / 1000) % 60;
        int ms = totalMs % 1000;

        std::wostringstream oss;
        oss << std::setfill(L'0') << std::setw(2) << minutes << L":"
            << std::setw(2) << secs << L"."
            << std::setw(3) << ms;
        return oss.str();
    }

    void MazeGameManager::UpdateUI() {
        if (m_timerText) {
            m_timerText->SetText(L"Time: " + FormatTime(m_elapsedTime));
        }

        if (m_countText) {
            std::wostringstream oss;
            oss << L"Collected: " << m_collectedCount << L" / " << m_totalBalls;
            m_countText->SetText(oss.str());
        }
    }

    void MazeGameManager::OnGameCleared() {
        m_state = MazeGameState::Cleared;

        if (m_clearText) {
            std::wostringstream oss;
            oss << L"CLEAR!\nTime: " << FormatTime(m_elapsedTime);
            m_clearText->SetText(oss.str());
        }

        if (m_restartText) {
            m_restartText->SetText(L"Press R to Restart");
        }
    }

} // namespace Game
