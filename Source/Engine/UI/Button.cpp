/// @file   Button.cpp
/// @brief  ボタンUI要素実装
#include "Button.h"

#include "Engine/Core/Logger.h"
#include "Engine/Graphics/RenderSystem.h"

namespace Engine {

    //============================================================
    // Lifecycle
    //============================================================

    void Button::Initialize() {
        UIElement::Initialize();
        m_state = m_isInteractable ? ButtonState::Normal : ButtonState::Disabled;
    }

    void Button::Update(float deltaTime) {
        UIElement::Update(deltaTime);

        // 無効状態の場合は常にDisabled
        if (!m_isInteractable) {
            m_state = ButtonState::Disabled;
        }
    }

    void Button::Render(RenderSystem* renderSystem) {
        if (!renderSystem || !m_isVisible) return;

        // 注: 実際のボタン描画は以下の手順で実装する
        // 1. 背景矩形の描画（現在の状態に応じた色で）
        // 2. テキストの描画（中央揃え）
        //
        // 実装例:
        // - SpriteBatch を使用した矩形描画
        // - DirectWrite を使用したテキスト描画
        // - または、頂点バッファを使用した矩形描画

        (void)renderSystem;
    }

    void Button::OnDestroy() {
        UIElement::OnDestroy();
        m_text.clear();
        m_onClick = nullptr;
    }

    //============================================================
    // Text
    //============================================================

    void Button::SetText(const std::wstring& text) {
        m_text = text;
    }

    const std::wstring& Button::GetText() const {
        return m_text;
    }

    void Button::SetFontSize(float fontSize) {
        if (fontSize > 0.0f) {
            m_fontSize = fontSize;
        }
    }

    float Button::GetFontSize() const {
        return m_fontSize;
    }

    //============================================================
    // Colors
    //============================================================

    void Button::SetNormalColor(const Color& color) {
        m_normalColor = color;
    }

    const Color& Button::GetNormalColor() const {
        return m_normalColor;
    }

    void Button::SetHoveredColor(const Color& color) {
        m_hoveredColor = color;
    }

    const Color& Button::GetHoveredColor() const {
        return m_hoveredColor;
    }

    void Button::SetPressedColor(const Color& color) {
        m_pressedColor = color;
    }

    const Color& Button::GetPressedColor() const {
        return m_pressedColor;
    }

    void Button::SetDisabledColor(const Color& color) {
        m_disabledColor = color;
    }

    const Color& Button::GetDisabledColor() const {
        return m_disabledColor;
    }

    void Button::SetTextColor(const Color& color) {
        m_textColor = color;
    }

    const Color& Button::GetTextColor() const {
        return m_textColor;
    }

    //============================================================
    // Interaction
    //============================================================

    void Button::SetOnClick(ClickCallback callback) {
        m_onClick = std::move(callback);
    }

    void Button::SetInteractable(bool interactable) {
        m_isInteractable = interactable;
        if (!interactable) {
            m_state = ButtonState::Disabled;
        }
        else if (m_state == ButtonState::Disabled) {
            m_state = ButtonState::Normal;
        }
    }

    bool Button::IsInteractable() const {
        return m_isInteractable;
    }

    Button::ButtonState Button::GetButtonState() const {
        return m_state;
    }

    //============================================================
    // Input Handling
    //============================================================

    bool Button::ContainsPoint(float mouseX, float mouseY) const {
        return mouseX >= m_position.m_x &&
               mouseX <= m_position.m_x + m_size.m_x &&
               mouseY >= m_position.m_y &&
               mouseY <= m_position.m_y + m_size.m_y;
    }

    void Button::HandleMouseInput(float mouseX, float mouseY, bool isPressed) {
        if (!m_isInteractable || !m_isEnabled) {
            m_state = ButtonState::Disabled;
            m_wasPressed = false;
            return;
        }

        bool isInside = ContainsPoint(mouseX, mouseY);

        if (isInside) {
            if (isPressed) {
                m_state = ButtonState::Pressed;
            }
            else {
                // マウスが押されていた状態から離された場合、クリックとして扱う
                if (m_wasPressed && m_onClick) {
                    m_onClick();
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
    // Protected
    //============================================================

    const Color& Button::GetCurrentBackgroundColor() const {
        switch (m_state) {
        case ButtonState::Hovered:
            return m_hoveredColor;
        case ButtonState::Pressed:
            return m_pressedColor;
        case ButtonState::Disabled:
            return m_disabledColor;
        case ButtonState::Normal:
        default:
            return m_normalColor;
        }
    }

} // namespace Engine
