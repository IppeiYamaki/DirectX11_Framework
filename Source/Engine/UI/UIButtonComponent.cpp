/// @file   UIButtonComponent.cpp
/// @brief  UIボタンコンポーネント実装
#include "UIButtonComponent.h"

#include "Engine/Core/Logger.h"

namespace Engine {

    //============================================================
    // Interaction
    //============================================================

    void UIButtonComponent::SetOnClick(ClickCallback callback) {
        m_onClick = std::move(callback);
    }

    void UIButtonComponent::OnClick() {
        if (m_isInteractable && m_onClick) {
            m_onClick();
        }
    }

    void UIButtonComponent::SetInteractable(bool interactable) {
        m_isInteractable = interactable;
        if (!interactable) {
            m_state = ButtonState::Disabled;
        }
        else if (m_state == ButtonState::Disabled) {
            m_state = ButtonState::Normal;
        }
    }

    bool UIButtonComponent::IsInteractable() const {
        return m_isInteractable;
    }

    //============================================================
    // State
    //============================================================

    UIButtonComponent::ButtonState UIButtonComponent::GetButtonState() const {
        return m_state;
    }

    void UIButtonComponent::SetButtonState(ButtonState state) {
        m_state = state;
    }

    //============================================================
    // Input Handling
    //============================================================

    void UIButtonComponent::HandleMouseInput(bool isInside, bool isPressed) {
        if (!m_isInteractable || !IsEnabled()) {
            m_state = ButtonState::Disabled;
            m_wasPressed = false;
            return;
        }

        if (isInside) {
            if (isPressed) {
                m_state = ButtonState::Pressed;
            }
            else {
                // マウスが押されていた状態から離された場合、クリックとして扱う
                if (m_wasPressed) {
                    OnClick();
                }
                m_state = ButtonState::Hovered;
            }
        }
        else {
            m_state = ButtonState::Normal;
        }

        m_wasPressed = isPressed && isInside;
    }

    //============================================================
    // Lifecycle
    //============================================================

    void UIButtonComponent::OnDestroy() {
        m_onClick = nullptr;
    }

} // namespace Engine
