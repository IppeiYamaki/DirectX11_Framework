/// @file   SunCycleComponent.h
/// @brief  太陽光の日周変化コンポーネント
#pragma once

#include "Engine/Scene/Component.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Scene/Components/LightComponent.h"

namespace Game {

    /// @brief 太陽光の日周変化エフェクト
    /// @note  DirectionalLightの方向・色・強度を時間経過で変化させる
    class SunCycleComponent final : public Engine::Component {
    public:
        SunCycleComponent();
        ~SunCycleComponent() override = default;

        void OnAwake() override;
        void Update(float deltaTime) override;

        //============================================================
        // 設定
        //============================================================

        /// @brief 1日のサイクル時間を設定（秒）
        void SetDayCycleDuration(float duration) { m_dayCycleDuration = duration; }

        /// @brief 現在の時刻を設定（0〜1、0=真夜中、0.5=正午）
        void SetTimeOfDay(float time) { m_timeOfDay = time; }

        /// @brief 現在の時刻を取得
        [[nodiscard]] float GetTimeOfDay() const { return m_timeOfDay; }

        /// @brief サイクルを一時停止
        void Pause() { m_isPaused = true; }

        /// @brief サイクルを再開
        void Resume() { m_isPaused = false; }

        /// @brief サイクルが一時停止中か確認
        [[nodiscard]] bool IsPaused() const { return m_isPaused; }

        //============================================================
        // 色温度設定
        //============================================================

        /// @brief 朝焼けの色を設定
        void SetDawnColor(const Engine::Vector3& color) { m_dawnColor = color; }

        /// @brief 正午の色を設定
        void SetNoonColor(const Engine::Vector3& color) { m_noonColor = color; }

        /// @brief 夕焼けの色を設定
        void SetDuskColor(const Engine::Vector3& color) { m_duskColor = color; }

        /// @brief 夜の色を設定
        void SetNightColor(const Engine::Vector3& color) { m_nightColor = color; }

        //============================================================
        // 強度設定
        //============================================================

        /// @brief 正午の強度を設定
        void SetNoonIntensity(float intensity) { m_noonIntensity = intensity; }

        /// @brief 夜の強度を設定
        void SetNightIntensity(float intensity) { m_nightIntensity = intensity; }

    private:
        /// @brief 時刻から太陽の方向を計算
        Engine::Vector3 CalculateSunDirection() const;

        /// @brief 時刻から太陽の色を計算
        Engine::Vector3 CalculateSunColor() const;

        /// @brief 時刻から太陽の強度を計算
        float CalculateSunIntensity() const;

    private:
        Engine::DirectionalLightComponent* m_lightComponent = nullptr;

        // 時間設定
        float m_dayCycleDuration = 120.0f;  // 2分で1日
        float m_timeOfDay = 0.5f;           // 0〜1（0=真夜中、0.5=正午）
        bool m_isPaused = false;

        // 色温度（デフォルト値）
        Engine::Vector3 m_dawnColor{ 1.0f, 0.6f, 0.4f };   // オレンジがかった朝焼け
        Engine::Vector3 m_noonColor{ 1.0f, 1.0f, 0.95f };  // 暖かい白色
        Engine::Vector3 m_duskColor{ 1.0f, 0.5f, 0.3f };   // 赤みがかった夕焼け
        Engine::Vector3 m_nightColor{ 0.3f, 0.4f, 0.6f };  // 青みがかった月光

        // 強度
        float m_noonIntensity = 1.5f;
        float m_nightIntensity = 0.2f;
    };

} // namespace Game
