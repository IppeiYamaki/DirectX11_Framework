/// @file   DebugCameraController.cpp
/// @brief  デバッグモード時のフリールックカメラコントローラー実装
#include "DebugCameraController.h"

#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Platform/Input.h"
#include "Engine/Core/Logger.h"

#include <cmath>
#include <algorithm>

namespace Engine {

    void DebugCameraController::OnStart() {
        auto* owner = GetOwner();
        if (owner) {
            m_transform = owner->GetComponent<Transform>();
            
            // 現在のTransformからYaw/Pitchを初期化
            if (m_transform) {
                const Vector3& rotation = m_transform->GetRotationEulerDegrees();
                m_yaw = rotation.y;
                m_pitch = rotation.x;
            }
        }
    }

    void DebugCameraController::Update(float deltaTime) {
        if (!m_isControllerEnabled || !m_transform) {
            m_isControlling = false;
            return;
        }

        // 右クリック状態をチェック
        bool rightMouseDown = Input::IsMousePressed(Input::MouseButton::Right);

        if (rightMouseDown) {
            m_isControlling = true;
            
            // 視点回転を処理
            ProcessRotation(deltaTime);
            
            // 移動を処理
            ProcessMovement(deltaTime);
        } else {
            m_isControlling = false;
        }
    }

    //============================================================
    // Enable/Disable
    //============================================================

    void DebugCameraController::Enable() {
        m_isControllerEnabled = true;
    }

    void DebugCameraController::Disable() {
        m_isControllerEnabled = false;
        m_isControlling = false;
    }

    bool DebugCameraController::IsControllerEnabled() const {
        return m_isControllerEnabled;
    }

    void DebugCameraController::ToggleEnabled() {
        if (m_isControllerEnabled) {
            Disable();
        } else {
            Enable();
        }
    }

    //============================================================
    // Settings
    //============================================================

    DebugCameraSettings& DebugCameraController::GetSettings() {
        return m_settings;
    }

    const DebugCameraSettings& DebugCameraController::GetSettings() const {
        return m_settings;
    }

    void DebugCameraController::SetMouseSensitivity(float sensitivity) {
        m_settings.m_mouseSensitivity = sensitivity;
    }

    void DebugCameraController::SetMoveSpeed(float speed) {
        m_settings.m_moveSpeed = speed;
    }

    //============================================================
    // State Query
    //============================================================

    bool DebugCameraController::IsControlling() const {
        return m_isControlling;
    }

    //============================================================
    // Private Methods
    //============================================================

    void DebugCameraController::ProcessRotation(float /*deltaTime*/) {
        // マウス移動量を取得
        POINT mouseDelta = Input::GetMouseDelta();
        
        // マウス移動量が0なら何もしない
        if (mouseDelta.x == 0 && mouseDelta.y == 0) {
            return;
        }

        // Yaw（水平回転）とPitch（垂直回転）を更新
        // 右方向移動でYawを増加、下方向移動でPitchを増加
        m_yaw += static_cast<float>(mouseDelta.x) * m_settings.m_mouseSensitivity;
        m_pitch += static_cast<float>(mouseDelta.y) * m_settings.m_mouseSensitivity;

        // Pitch角度を制限（真上・真下を向かないように）
        m_pitch = (std::max)(-m_settings.m_pitchLimit, (std::min)(m_settings.m_pitchLimit, m_pitch));

        // Yawを0-360度の範囲に正規化
        while (m_yaw > 360.0f) m_yaw -= 360.0f;
        while (m_yaw < 0.0f) m_yaw += 360.0f;

        // Transformに回転を適用
        m_transform->SetYawPitchRollDegrees(m_yaw, m_pitch, 0.0f);
    }

    void DebugCameraController::ProcessMovement(float deltaTime) {
        // 移動方向を計算
        Vector3 moveDirection(0.0f, 0.0f, 0.0f);

        // WASDで前後左右移動
        if (Input::IsKeyPressed('W')) {
            moveDirection.z += 1.0f;  // 前進
        }
        if (Input::IsKeyPressed('S')) {
            moveDirection.z -= 1.0f;  // 後退
        }
        if (Input::IsKeyPressed('A')) {
            moveDirection.x -= 1.0f;  // 左移動
        }
        if (Input::IsKeyPressed('D')) {
            moveDirection.x += 1.0f;  // 右移動
        }
        
        // Q/Eで上下移動
        if (Input::IsKeyPressed('Q')) {
            moveDirection.y -= 1.0f;  // 下降
        }
        if (Input::IsKeyPressed('E')) {
            moveDirection.y += 1.0f;  // 上昇
        }

        // 移動量がなければ終了
        float lengthSq = moveDirection.x * moveDirection.x + 
                         moveDirection.y * moveDirection.y + 
                         moveDirection.z * moveDirection.z;
        if (lengthSq < 0.0001f) {
            return;
        }

        // 移動方向を正規化
        float invLength = 1.0f / std::sqrt(lengthSq);
        moveDirection.x *= invLength;
        moveDirection.y *= invLength;
        moveDirection.z *= invLength;

        // 移動速度を計算（Shift押下で高速移動）
        float speed = m_settings.m_moveSpeed;
        if (Input::IsKeyPressed(VK_SHIFT)) {
            speed *= m_settings.m_fastMoveMultiplier;
        }

        // カメラのローカル座標系での移動量を計算
        Vector3 forward = m_transform->GetForward();
        Vector3 right = m_transform->GetRight();
        Vector3 up(0.0f, 1.0f, 0.0f);  // ワールド上方向を使用

        // 移動ベクトルを計算
        Vector3 movement(0.0f, 0.0f, 0.0f);
        movement.x = right.x * moveDirection.x + forward.x * moveDirection.z;
        movement.y = moveDirection.y;  // Y移動はワールド座標系で直接
        movement.z = right.z * moveDirection.x + forward.z * moveDirection.z;

        // 移動量を適用
        float moveAmount = speed * deltaTime;
        m_transform->AddPosition(
            movement.x * moveAmount,
            movement.y * moveAmount,
            movement.z * moveAmount
        );
    }

} // namespace Engine
