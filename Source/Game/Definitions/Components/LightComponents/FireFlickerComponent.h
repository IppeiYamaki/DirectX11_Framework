/// @file   FireFlickerComponent.h
/// @brief  炎の揺らぎエフェクト用コンポーネント
#pragma once

#include "Engine/Scene/Component.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Scene/Components/LightComponent.h"

namespace Game {

    /// @brief 炎の揺らぎエフェクト
    /// @note  PointLightの強度・色・位置をランダムに変動させて炎のような効果を出す
    class FireFlickerComponent final : public Engine::Component {
    public:
        FireFlickerComponent();
        ~FireFlickerComponent() override = default;

        void OnAwake() override;
        void Update(float deltaTime) override;

        //============================================================
        // 設定
        //============================================================

        /// @brief 基準となる光の強度を設定
        void SetBaseIntensity(float intensity) { m_baseIntensity = intensity; }

        /// @brief 強度の揺らぎ幅を設定（0〜1、基準強度に対する割合）
        void SetIntensityFlickerRange(float range) { m_intensityFlickerRange = range; }

        /// @brief 揺らぎの速度を設定
        void SetFlickerSpeed(float speed) { m_flickerSpeed = speed; }

        /// @brief 位置の揺らぎ幅を設定
        void SetPositionFlickerRange(float range) { m_positionFlickerRange = range; }

        /// @brief 基準位置を設定
        void SetBasePosition(const Engine::Vector3& pos) { m_basePosition = pos; m_basePositionSet = true; }

        /// @brief 基準色を設定
        void SetBaseColor(const Engine::Vector3& color) { m_baseColor = color; }

        /// @brief 色の揺らぎ範囲を設定（暖色の範囲で変動）
        void SetColorFlickerRange(float range) { m_colorFlickerRange = range; }

    private:
        Engine::PointLightComponent* m_lightComponent = nullptr;

        // 基準値
        Engine::Vector3 m_basePosition{ 0.0f, 0.0f, 0.0f };
        Engine::Vector3 m_baseColor{ 1.0f, 0.6f, 0.2f };  // オレンジ色
        float m_baseIntensity = 2.0f;

        // 揺らぎパラメータ
        float m_intensityFlickerRange = 0.3f;  // 強度の30%で揺らぐ
        float m_positionFlickerRange = 0.1f;   // 0.1ユニットで揺らぐ
        float m_colorFlickerRange = 0.1f;      // 色の10%で揺らぐ
        float m_flickerSpeed = 8.0f;           // 揺らぎ速度

        // 内部状態
        float m_elapsedTime = 0.0f;
        float m_randomSeed = 0.0f;
        bool m_basePositionSet = false;        // 基準位置が明示的に設定されたか
    };

} // namespace Game
