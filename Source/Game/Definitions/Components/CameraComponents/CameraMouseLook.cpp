/// @file   CameraMouseLook.cpp
/// @brief  マウスでカメラを回転させるコンポーネント実装
#include "CameraMouseLook.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Platform/Input.h"

#include <algorithm>

namespace Game {

    CameraMouseLook::CameraMouseLook(float sensitivity)
        : m_sensitivity(sensitivity)
        , m_minPitch(-89.0f)
        , m_maxPitch(89.0f)
        , m_currentYaw(0.0f)
        , m_currentPitch(0.0f)
    {
    }

    void CameraMouseLook::Update(float deltaTime) {
        (void)deltaTime; // deltaTimeは使用しない（マウス移動量ベース）

        auto* owner = GetOwner();
        if (!owner) return;

        auto* transform = owner->GetComponent<Engine::Transform>();
        if (!transform) return;

        // マウス移動量を取得
        POINT mouseDelta = Engine::Input::GetMouseDelta();

        // 右クリック中のみ回転させる
        if (Engine::Input::IsMousePressed(Engine::Input::MouseButton::Right)) {
            // ヨーとピッチを計算
            m_currentYaw += static_cast<float>(mouseDelta.x) * m_sensitivity;
            m_currentPitch -= static_cast<float>(mouseDelta.y) * m_sensitivity;

            // ピッチを制限
            m_currentPitch = std::clamp(m_currentPitch, m_minPitch, m_maxPitch);

            // 回転を適用
            transform->SetYawPitchRollDegrees(m_currentYaw, m_currentPitch, 0.0f);
        }
    }

    void CameraMouseLook::SetSensitivity(float sensitivity) {
        m_sensitivity = sensitivity;
    }

    float CameraMouseLook::GetSensitivity() const {
        return m_sensitivity;
    }

    void CameraMouseLook::SetPitchLimits(float minPitch, float maxPitch) {
        m_minPitch = minPitch;
        m_maxPitch = maxPitch;
    }

} // namespace Game
