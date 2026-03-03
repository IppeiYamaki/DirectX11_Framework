/// @file   UIRectTransform.cpp
/// @brief  UI用RectTransformコンポーネント実装
#include "UIRectTransform.h"

namespace Engine {

    //============================================================
    // Position / Size
    //============================================================

    void UIRectTransform::SetPosition(const Vector2& position) {
        m_position = position;
    }

    void UIRectTransform::SetPosition(float x, float y) {
        m_position.x = x;
        m_position.y = y;
    }

    const Vector2& UIRectTransform::GetPosition() const {
        return m_position;
    }

    void UIRectTransform::SetSize(const Vector2& size) {
        m_size = size;
    }

    void UIRectTransform::SetSize(float width, float height) {
        m_size.x = width;
        m_size.y = height;
    }

    const Vector2& UIRectTransform::GetSize() const {
        return m_size;
    }

    //============================================================
    // Anchor / Pivot
    //============================================================

    void UIRectTransform::SetAnchor(const Vector2& anchor) {
        m_anchor = anchor;
    }

    const Vector2& UIRectTransform::GetAnchor() const {
        return m_anchor;
    }

    void UIRectTransform::SetPivot(const Vector2& pivot) {
        m_pivot = pivot;
    }

    const Vector2& UIRectTransform::GetPivot() const {
        return m_pivot;
    }

    //============================================================
    // Computed Rect
    //============================================================

    void UIRectTransform::GetRect(float& outLeft, float& outTop, float& outRight, float& outBottom) const {
        // ピボットを考慮した左上座標を計算
        float pivotOffsetX = m_size.x * m_pivot.x;
        float pivotOffsetY = m_size.y * m_pivot.y;

        outLeft = m_position.x - pivotOffsetX;
        outTop = m_position.y - pivotOffsetY;
        outRight = outLeft + m_size.x;
        outBottom = outTop + m_size.y;
    }

    bool UIRectTransform::Contains(float x, float y) const {
        float left, top, right, bottom;
        GetRect(left, top, right, bottom);

        return (x >= left && x <= right && y >= top && y <= bottom);
    }

    //============================================================
    // Layer
    //============================================================

    void UIRectTransform::SetSortOrder(int order) {
        m_sortOrder = order;
    }

    int UIRectTransform::GetSortOrder() const {
        return m_sortOrder;
    }

} // namespace Engine
