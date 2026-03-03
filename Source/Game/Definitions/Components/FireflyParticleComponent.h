#pragma once

#include "Engine/Scene/Component.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Color.h"

namespace Game {

    /**
     * @brief 蛍パーティクルコンポーネント
     * - 漂い動き（sin/ノイズによる曲線運動）
     * - 明滅（sin/timeによる輝度変化）
     * - 各パーティクルは独立したパラメータを持つ
     */
    class FireflyParticleComponent final : public Engine::Component {
    public:
        FireflyParticleComponent();

        void OnStart() override;
        void Update(float deltaTime) override;

        /// @brief 初期位置を設定
        void SetBasePosition(const Engine::Vector3& pos) { m_basePosition = pos; }

        /// @brief 漂い範囲を設定
        void SetDriftRange(const Engine::Vector3& range) { m_driftRange = range; }

        /// @brief 漂い速度を設定
        void SetDriftSpeed(float speed) { m_driftSpeed = speed; }

        /// @brief 明滅速度を設定
        void SetFlickerSpeed(float speed) { m_flickerSpeed = speed; }

        /// @brief 明滅の最小輝度を設定（0.0～1.0）
        void SetMinBrightness(float brightness) { m_minBrightness = brightness; }

        /// @brief ベースカラーを設定
        void SetBaseColor(const Engine::Color& color) { m_baseColor = color; }

        /// @brief パーティクルサイズを設定
        void SetSize(float size) { m_size = size; }

        /// @brief パーティクルサイズを取得
        float GetSize() const { return m_size; }

        /// @brief 現在の輝度を取得（0.0～1.0）
        float GetBrightness() const { return m_currentBrightness; }

        /// @brief 現在の色を取得
        Engine::Color GetCurrentColor() const;

    private:
        Engine::Vector3 m_basePosition;         // 基準位置
        Engine::Vector3 m_driftRange;           // 漂い範囲（XYZ各方向）
        float m_driftSpeed;                     // 漂い速度
        float m_flickerSpeed;                   // 明滅速度
        float m_minBrightness;                  // 最小輝度
        Engine::Color m_baseColor;              // ベースカラー
        float m_size;                           // パーティクルサイズ

        float m_elapsedTime;                    // 経過時間
        float m_phaseOffset;                    // 位相オフセット（ランダム）
        float m_noiseOffsetX;                   // ノイズ用オフセットX
        float m_noiseOffsetY;                   // ノイズ用オフセットY
        float m_noiseOffsetZ;                   // ノイズ用オフセットZ
        float m_flickerPhaseOffset;             // 明滅の位相オフセット

        float m_currentBrightness;              // 現在の輝度
    };

} // namespace Game
