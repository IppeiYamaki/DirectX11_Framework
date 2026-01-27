/// @file   UIElement.h
/// @brief  UI要素の基底クラス
#pragma once

#include "Engine/Math/Vector2.h"

namespace Engine {

    class Canvas;
    class RenderSystem;

    /// @brief UI要素の基底クラス
    /// @note  各種UI要素（ボタン、テキストなど）はこのクラスを継承する
    class UIElement {
    public:
        UIElement() = default;
        virtual ~UIElement() = default;

        UIElement(const UIElement&) = delete;
        UIElement& operator=(const UIElement&) = delete;

        //============================================================
        // Lifecycle
        //============================================================

        /// @brief 初期化
        virtual void Initialize();

        /// @brief 毎フレーム更新
        /// @param deltaTime フレーム経過時間
        virtual void Update(float deltaTime);

        /// @brief 描画
        /// @param renderSystem RenderSystem参照
        virtual void Render(RenderSystem* renderSystem);

        /// @brief 破棄時処理
        virtual void OnDestroy();

        //============================================================
        // Transform
        //============================================================

        /// @brief スクリーン座標での位置を設定
        /// @param position 位置（ピクセル単位）
        void SetPosition(const Vector2& position);

        /// @brief スクリーン座標での位置を取得
        /// @return 位置（ピクセル単位）
        [[nodiscard]] const Vector2& GetPosition() const;

        /// @brief サイズを設定
        /// @param size サイズ（ピクセル単位）
        void SetSize(const Vector2& size);

        /// @brief サイズを取得
        /// @return サイズ（ピクセル単位）
        [[nodiscard]] const Vector2& GetSize() const;

        //============================================================
        // State
        //============================================================

        /// @brief 有効化
        void Enable();

        /// @brief 無効化
        void Disable();

        /// @brief 有効か確認
        /// @return 有効ならtrue
        [[nodiscard]] bool IsEnabled() const;

        /// @brief 表示設定
        /// @param visible 表示するか
        void SetVisible(bool visible);

        /// @brief 表示しているか確認
        /// @return 表示していればtrue
        [[nodiscard]] bool IsVisible() const;

        //============================================================
        // Layer
        //============================================================

        /// @brief 描画順序を設定（大きいほど手前）
        /// @param order 描画順序
        void SetSortOrder(int order);

        /// @brief 描画順序を取得
        /// @return 描画順序
        [[nodiscard]] int GetSortOrder() const;

        //============================================================
        // Canvas
        //============================================================

        /// @brief 所属Canvasを設定
        /// @param canvas Canvas
        void SetCanvas(Canvas* canvas);

        /// @brief 所属Canvasを取得
        /// @return Canvas
        [[nodiscard]] Canvas* GetCanvas() const;

        //============================================================
        // Hit Testing / Click
        //============================================================

        /// @brief 指定座標がこの要素の領域内にあるか判定
        /// @param x スクリーンX座標
        /// @param y スクリーンY座標
        /// @return 領域内ならtrue
        [[nodiscard]] virtual bool Contains(float x, float y) const;

        /// @brief クリック時に呼ばれるコールバック
        /// @note  派生クラスでオーバーライドして独自のクリック処理を実装可能
        virtual void OnClick();

    protected:
        Vector2 m_position{ 0.0f, 0.0f };   ///< スクリーン座標での位置
        Vector2 m_size{ 100.0f, 100.0f };   ///< サイズ
        bool m_isEnabled = true;            ///< 有効フラグ
        bool m_isVisible = true;            ///< 表示フラグ
        int m_sortOrder = 0;                ///< 描画順序
        Canvas* m_canvas = nullptr;         ///< 所属Canvas（借用）
    };

} // namespace Engine
