/// @file   CameraMouseLook.h
/// @brief  マウスでカメラを回転させるコンポーネント
#pragma once

#include "Engine/Scene/Component.h"

namespace Game {

    /// @brief マウスでカメラを回転させるコンポーネント
    class CameraMouseLook : public Engine::Component {
    public:
        /// @brief コンストラクタ
        /// @param sensitivity マウス感度（デフォルト: 0.2）
        explicit CameraMouseLook(float sensitivity = 0.2f);
        
        ~CameraMouseLook() override = default;

        /// @brief 毎フレーム更新
        /// @param deltaTime 前フレームからの経過時間
        void Update(float deltaTime) override;

        /// @brief マウス感度を設定
        /// @param sensitivity マウス感度
        void SetSensitivity(float sensitivity);

        /// @brief マウス感度を取得
        /// @return 現在のマウス感度
        [[nodiscard]] float GetSensitivity() const;

        /// @brief ピッチ制限を設定
        /// @param minPitch 最小ピッチ角度（度）
        /// @param maxPitch 最大ピッチ角度（度）
        void SetPitchLimits(float minPitch, float maxPitch);

    private:
        float m_sensitivity;  ///< マウス感度
        float m_minPitch;     ///< 最小ピッチ角度
        float m_maxPitch;     ///< 最大ピッチ角度
        float m_currentYaw;   ///< 現在のヨー角
        float m_currentPitch; ///< 現在のピッチ角
    };

} // namespace Game
