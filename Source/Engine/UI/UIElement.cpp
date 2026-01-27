/// @file   UIElement.cpp
/// @brief  UI要素の基底クラス実装
#include "UIElement.h"

namespace Engine {

    //============================================================
    // Lifecycle
    //============================================================

    void UIElement::Initialize() {
        // 派生クラスでオーバーライド
    }

    void UIElement::Update(float deltaTime) {
        (void)deltaTime;
        // 派生クラスでオーバーライド
    }

    void UIElement::Render(RenderSystem* renderSystem) {
        (void)renderSystem;
        // 派生クラスでオーバーライド
    }

    void UIElement::OnDestroy() {
        // 派生クラスでオーバーライド
    }

    //============================================================
    // Transform
    //============================================================

    void UIElement::SetPosition(const Vector2& position) {
        m_position = position;
    }

    const Vector2& UIElement::GetPosition() const {
        return m_position;
    }

    void UIElement::SetSize(const Vector2& size) {
        m_size = size;
    }

    const Vector2& UIElement::GetSize() const {
        return m_size;
    }

    //============================================================
    // State
    //============================================================

    void UIElement::Enable() {
        m_isEnabled = true;
    }

    void UIElement::Disable() {
        m_isEnabled = false;
    }

    bool UIElement::IsEnabled() const {
        return m_isEnabled;
    }

    void UIElement::SetVisible(bool visible) {
        m_isVisible = visible;
    }

    bool UIElement::IsVisible() const {
        return m_isVisible;
    }

    //============================================================
    // Layer
    //============================================================

    void UIElement::SetSortOrder(int order) {
        m_sortOrder = order;
    }

    int UIElement::GetSortOrder() const {
        return m_sortOrder;
    }

    //============================================================
    // Canvas
    //============================================================

    void UIElement::SetCanvas(Canvas* canvas) {
        m_canvas = canvas;
    }

    Canvas* UIElement::GetCanvas() const {
        return m_canvas;
    }

    //============================================================
    // Hit Testing / Click
    //============================================================

    bool UIElement::Contains(float x, float y) const {
        // 矩形領域内判定
        float left = m_position.x;
        float top = m_position.y;
        float right = m_position.x + m_size.x;
        float bottom = m_position.y + m_size.y;

        return (x >= left && x <= right && y >= top && y <= bottom);
    }

    void UIElement::OnClick() {
        // 派生クラスでオーバーライド
    }

} // namespace Engine
