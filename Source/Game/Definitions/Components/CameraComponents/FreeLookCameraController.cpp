/// @file   FreeLookCameraController.cpp
/// @brief  右クリック＋マウスドラッグでカメラの視点を回転させるコンポーネントの実装
#include "FreeLookCameraController.h"

#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Platform/Input.h"

#include <algorithm>
#include <cmath>

namespace Game {

    FreeLookCameraController::FreeLookCameraController(float mouseSensitivity)
        : m_transform(nullptr)
        , m_isLooking(false)
        , m_yaw(0.0f)
        , m_pitch(0.0f)
    {
        m_settings.m_mouseSensitivity = mouseSensitivity;
    }

    //============================================================
    // ライフサイクル
    //============================================================

    void FreeLookCameraController::OnStart() {
        auto* owner = GetOwner();
        if (owner) {
            m_transform = owner->GetComponent<Engine::Transform>();

            // 現在のTransformからYaw/Pitchを初期化
            if (m_transform) {
                const Engine::Vector3& rotation = m_transform->GetRotationEulerDegrees();
                m_yaw = rotation.y;
                m_pitch = rotation.x;
            }
        }
    }

    void FreeLookCameraController::Update(float /*deltaTime*/) {
        if (!m_transform) {
            m_isLooking = false;
            return;
        }

        // 右クリック状態をチェック
        bool rightMouseDown = Engine::Input::IsMousePressed(Engine::Input::MouseButton::Right);

        if (rightMouseDown) {
            m_isLooking = true;

            // マウス移動量を取得
            POINT mouseDelta = Engine::Input::GetMouseDelta();

            // マウス移動量が0なら何もしない
            if (mouseDelta.x == 0 && mouseDelta.y == 0) {
                return;
            }

            // Yaw（水平回転）とPitch（垂直回転）を更新
            // 右方向移動でYawを増加、下方向移動でPitchを増加
            m_yaw += static_cast<float>(mouseDelta.x) * m_settings.m_mouseSensitivity;
            m_pitch += static_cast<float>(mouseDelta.y) * m_settings.m_mouseSensitivity;

            // Pitch角度を制限（真上・真下を向かないように）
            m_pitch = std::clamp(m_pitch, m_settings.m_pitchLimitMin, m_settings.m_pitchLimitMax);

            // Yawを0-360度の範囲に正規化（fmodで効率的に）
            m_yaw = std::fmod(m_yaw, 360.0f);
            if (m_yaw < 0.0f) m_yaw += 360.0f;

            // Transformに回転を適用
            m_transform->SetYawPitchRollDegrees(m_yaw, m_pitch, 0.0f);
        }
        else {
            m_isLooking = false;
        }
    }

    //============================================================
    // 設定
    //============================================================

    FreeLookCameraSettings& FreeLookCameraController::GetSettings() {
        return m_settings;
    }

    const FreeLookCameraSettings& FreeLookCameraController::GetSettings() const {
        return m_settings;
    }

    void FreeLookCameraController::SetMouseSensitivity(float sensitivity) {
        m_settings.m_mouseSensitivity = sensitivity;
    }

    float FreeLookCameraController::GetMouseSensitivity() const {
        return m_settings.m_mouseSensitivity;
    }

    void FreeLookCameraController::SetPitchLimits(float minPitch, float maxPitch) {
        m_settings.m_pitchLimitMin = minPitch;
        m_settings.m_pitchLimitMax = maxPitch;
    }

    //============================================================
    // 状態クエリ
    //============================================================

    bool FreeLookCameraController::IsLooking() const {
        return m_isLooking;
    }

    float FreeLookCameraController::GetCurrentYaw() const {
        return m_yaw;
    }

    float FreeLookCameraController::GetCurrentPitch() const {
        return m_pitch;
    }

} // namespace Game
