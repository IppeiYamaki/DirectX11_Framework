/// @file   UIRectTransform.h
/// @brief  UI用RectTransformコンポーネント（Unity風）
#pragma once

#include "Engine/Scene/Component.h"
#include "Engine/Math/Vector2.h"

namespace Engine {

    /// @brief UI用RectTransformコンポーネント
    /// @note  UI要素の位置/サイズをスクリーン座標で保持し、アンカーとピボットをサポート
    class UIRectTransform : public Component {
    public:
        UIRectTransform() = default;
        ~UIRectTransform() override = default;

        //============================================================
        // Position / Size (スクリーン座標)
        //============================================================

        /// @brief 位置を設定（スクリーン座標）
        /// @param position 左上からの位置（ピクセル単位）
        void SetPosition(const Vector2& position);

        /// @brief 位置を設定（スクリーン座標）
        /// @param x X座標
        /// @param y Y座標
        void SetPosition(float x, float y);

        /// @brief 位置を取得（スクリーン座標）
        /// @return 位置
        [[nodiscard]] const Vector2& GetPosition() const;

        /// @brief サイズを設定
        /// @param size 幅と高さ（ピクセル単位）
        void SetSize(const Vector2& size);

        /// @brief サイズを設定
        /// @param width 幅
        /// @param height 高さ
        void SetSize(float width, float height);

        /// @brief サイズを取得
        /// @return サイズ
        [[nodiscard]] const Vector2& GetSize() const;

        //============================================================
        // Anchor / Pivot
        //============================================================

        /// @brief アンカーを設定（0.0～1.0、親に対する相対位置）
        /// @param anchor アンカー座標
        void SetAnchor(const Vector2& anchor);

        /// @brief アンカーを取得
        /// @return アンカー座標
        [[nodiscard]] const Vector2& GetAnchor() const;

        /// @brief ピボットを設定（0.0～1.0、自身の基準点）
        /// @param pivot ピボット座標
        void SetPivot(const Vector2& pivot);

        /// @brief ピボットを取得
        /// @return ピボット座標
        [[nodiscard]] const Vector2& GetPivot() const;

        //============================================================
        // Computed Rect (描画用)
        //============================================================

        /// @brief 矩形領域を取得（左上を基準としたスクリーン座標）
        /// @param outLeft 左端
        /// @param outTop 上端
        /// @param outRight 右端
        /// @param outBottom 下端
        void GetRect(float& outLeft, float& outTop, float& outRight, float& outBottom) const;

        /// @brief 指定座標がこのRectTransformの領域内にあるか判定
        /// @param x スクリーンX座標
        /// @param y スクリーンY座標
        /// @return 領域内ならtrue
        [[nodiscard]] bool Contains(float x, float y) const;

        //============================================================
        // Layer (描画順)
        //============================================================

        /// @brief 描画順序を設定（大きいほど手前）
        /// @param order 描画順序
        void SetSortOrder(int order);

        /// @brief 描画順序を取得
        /// @return 描画順序
        [[nodiscard]] int GetSortOrder() const;

    private:
        Vector2 m_position{ 0.0f, 0.0f };       ///< スクリーン座標での位置
        Vector2 m_size{ 100.0f, 100.0f };       ///< サイズ（ピクセル単位）
        Vector2 m_anchor{ 0.0f, 0.0f };         ///< アンカー（0.0～1.0）
        Vector2 m_pivot{ 0.0f, 0.0f };          ///< ピボット（0.0～1.0）
        int m_sortOrder = 0;                    ///< 描画順序
    };

} // namespace Engine
