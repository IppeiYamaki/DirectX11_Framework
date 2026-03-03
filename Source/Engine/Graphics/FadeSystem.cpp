/// @file   FadeSystem.cpp
/// @brief  シーン遷移・演出用のフェードシステム実装
#include "FadeSystem.h"

#include <algorithm>

#include "Engine/Core/Logger.h"
#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Math/Easing.h"

namespace Engine {

    FadeSystem::~FadeSystem() {
        Finalize();
    }

    //============================================================
    // Lifecycle
    //============================================================

    bool FadeSystem::Initialize(RenderSystem* renderSystem, float screenWidth, float screenHeight) {
        if (m_isInitialized) return true;

        if (!renderSystem) {
            Logger::Error("FadeSystem::Initialize failed: renderSystem is null.");
            return false;
        }

        m_renderSystem = renderSystem;
        m_screenWidth = screenWidth;
        m_screenHeight = screenHeight;

        // フェード用白テクスチャを作成（RenderSystemが1x1白テクスチャを提供するか、
        // または DrawSprite に nullptr を渡して単色描画する実装にする）
        // 現在の実装では白テクスチャがなくても色付きスプライトとして描画可能
        // フェード色はSetFadeColorで設定され、DrawSpriteのカラー乗算で適用される

        // 状態を初期化
        m_mode = FadeMode::None;
        m_state = FadeState::Idle;
        m_easing = EasingType::Linear;
        m_duration = 0.0f;
        m_elapsed = 0.0f;
        m_alpha = 0.0f;

        m_isInitialized = true;
        Logger::Info("FadeSystem initialized.");
        return true;
    }

    void FadeSystem::Finalize() {
        if (!m_isInitialized) return;

        m_renderSystem = nullptr;

        m_mode = FadeMode::None;
        m_state = FadeState::Idle;
        m_alpha = 0.0f;

        m_isInitialized = false;
        Logger::Info("FadeSystem finalized.");
    }

    //============================================================
    // Fade Control API
    //============================================================

    void FadeSystem::SetFade(FadeMode mode, EasingType easing, float duration) {
        if (!m_isInitialized) {
            Logger::Warn("FadeSystem::SetFade: System not initialized.");
            return;
        }

        if (mode == FadeMode::None) {
            ResetFade();
            return;
        }

        if (duration <= 0.0f) {
            Logger::Warn("FadeSystem::SetFade: Invalid duration. Using 0.5 seconds.");
            duration = 0.5f;
        }

        m_mode = mode;
        m_easing = easing;
        m_duration = duration;
        m_elapsed = 0.0f;
        m_state = FadeState::Running;

        // 初期アルファ値を設定
        if (mode == FadeMode::FadeIn) {
            // FadeIn: 画面が見えてくる（alpha: 1.0→0.0）
            m_alpha = 1.0f;
        } else if (mode == FadeMode::FadeOut) {
            // FadeOut: 画面が暗くなる（alpha: 0.0→1.0）
            m_alpha = 0.0f;
        }

        Logger::Info("FadeSystem: Started " + 
                     std::string(mode == FadeMode::FadeIn ? "FadeIn" : "FadeOut") +
                     " (duration: " + std::to_string(duration) + "s)");
    }

    float FadeSystem::GetFadeAlpha() const {
        return m_alpha;
    }

    bool FadeSystem::IsFadeDone() const {
        return m_state == FadeState::Done;
    }

    bool FadeSystem::IsFadeRunning() const {
        return m_state == FadeState::Running;
    }

    void FadeSystem::CancelFade() {
        if (m_state == FadeState::Running) {
            m_state = FadeState::Idle;
            m_mode = FadeMode::None;
            Logger::Info("FadeSystem: Fade cancelled.");
        }
    }

    void FadeSystem::ResetFade() {
        m_mode = FadeMode::None;
        m_state = FadeState::Idle;
        m_elapsed = 0.0f;
        m_alpha = 0.0f;
        Logger::Trace("FadeSystem: Fade reset.");
    }

    //============================================================
    // Frame
    //============================================================

    void FadeSystem::Update(float deltaTime) {
        if (!m_isInitialized) return;
        if (m_state != FadeState::Running) return;

        // 経過時間を更新
        m_elapsed += deltaTime;

        // 進行率を計算（0.0～1.0にクランプ）
        float progress = std::clamp(m_elapsed / m_duration, 0.0f, 1.0f);

        // イージングを適用
        float easedProgress = ApplyEasing(progress);

        // アルファ値を計算
        if (m_mode == FadeMode::FadeIn) {
            // FadeIn: 1.0→0.0
            m_alpha = 1.0f - easedProgress;
        } else if (m_mode == FadeMode::FadeOut) {
            // FadeOut: 0.0→1.0
            m_alpha = easedProgress;
        }

        // 完了判定
        if (progress >= 1.0f) {
            m_state = FadeState::Done;
            Logger::Info("FadeSystem: Fade completed.");
        }
    }

    void FadeSystem::Draw() {
        if (!m_isInitialized) return;
        if (!m_renderSystem) return;

        // アルファが0の場合は描画不要
        if (m_alpha <= 0.0001f) return;

        // 画面全体を覆う単色矩形を描画
        // RenderSystemのDrawColoredRectを使用して、フェード色×アルファで半透明オーバーレイを描画
        m_renderSystem->DrawColoredRect(
            0.0f, 0.0f,                         // 左上
            m_screenWidth, m_screenHeight,      // 右下
            m_fadeColorR, m_fadeColorG, m_fadeColorB, m_alpha
        );
    }

    //============================================================
    // Settings
    //============================================================

    void FadeSystem::SetFadeColor(float r, float g, float b) {
        m_fadeColorR = std::clamp(r, 0.0f, 1.0f);
        m_fadeColorG = std::clamp(g, 0.0f, 1.0f);
        m_fadeColorB = std::clamp(b, 0.0f, 1.0f);
    }

    void FadeSystem::SetScreenSize(float width, float height) {
        m_screenWidth = width;
        m_screenHeight = height;
    }

    //============================================================
    // State
    //============================================================

    bool FadeSystem::IsInitialized() const {
        return m_isInitialized;
    }

    FadeMode FadeSystem::GetCurrentMode() const {
        return m_mode;
    }

    FadeState FadeSystem::GetCurrentState() const {
        return m_state;
    }

    float FadeSystem::GetProgress() const {
        if (m_duration <= 0.0f) return 0.0f;
        return std::clamp(m_elapsed / m_duration, 0.0f, 1.0f);
    }

    //============================================================
    // Private
    //============================================================

    float FadeSystem::ApplyEasing(float t) const {
        // t は 0.0～1.0 の範囲
        switch (m_easing) {
            case EasingType::Linear:
                return t;

            case EasingType::EaseIn:
                return Easing::EaseInQuad(t);

            case EasingType::EaseOut:
                return Easing::EaseOutQuad(t);

            case EasingType::EaseInOut:
                return Easing::EaseInOutQuad(t);

            case EasingType::EaseInSine:
                return Easing::EaseInSine(t);

            case EasingType::EaseOutSine:
                return Easing::EaseOutSine(t);

            case EasingType::EaseInOutSine:
                return Easing::EaseInOutSine(t);

            case EasingType::EaseInCubic:
                return Easing::EaseInCubic(t);

            case EasingType::EaseOutCubic:
                return Easing::EaseOutCubic(t);

            case EasingType::EaseInOutCubic:
                return Easing::EaseInOutCubic(t);

            default:
                return t;
        }
    }

} // namespace Engine
