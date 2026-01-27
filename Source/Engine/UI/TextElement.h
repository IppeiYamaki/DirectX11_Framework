/// @file   TextElement.h
/// @brief  テキスト表示用UI要素
#pragma once

#include "Engine/UI/UIElement.h"
#include "Engine/Math/Color.h"

#include <string>

namespace Engine {

    /// @brief テキスト表示用UI要素
    /// @note  Canvas上にテキストを表示するためのUIコンポーネント
    class TextElement : public UIElement {
    public:
        /// @brief テキストの水平方向揃え位置
        enum class HorizontalAlignment {
            Left,
            Center,
            Right
        };

        /// @brief テキストの垂直方向揃え位置
        enum class VerticalAlignment {
            Top,
            Middle,
            Bottom
        };

    public:
        TextElement() = default;
        ~TextElement() override = default;

        //============================================================
        // Lifecycle
        //============================================================

        /// @brief 初期化
        void Initialize() override;

        /// @brief 毎フレーム更新
        /// @param deltaTime フレーム経過時間
        void Update(float deltaTime) override;

        /// @brief 描画
        /// @param renderSystem RenderSystem参照
        void Render(RenderSystem* renderSystem) override;

        /// @brief 破棄時処理
        void OnDestroy() override;

        //============================================================
        // Text
        //============================================================

        /// @brief テキストを設定
        /// @param text 表示するテキスト
        void SetText(const std::wstring& text);

        /// @brief テキストを取得
        /// @return 表示テキスト
        [[nodiscard]] const std::wstring& GetText() const;

        //============================================================
        // Font Properties
        //============================================================

        /// @brief フォントサイズを設定（ピクセル単位）
        /// @param fontSize フォントサイズ
        void SetFontSize(float fontSize);

        /// @brief フォントサイズを取得
        /// @return フォントサイズ
        [[nodiscard]] float GetFontSize() const;

        /// @brief フォント名を設定
        /// @param fontName フォント名
        void SetFontName(const std::wstring& fontName);

        /// @brief フォント名を取得
        /// @return フォント名
        [[nodiscard]] const std::wstring& GetFontName() const;

        //============================================================
        // Color
        //============================================================

        /// @brief テキストカラーを設定
        /// @param color テキストカラー（RGBA）
        void SetColor(const Color& color);

        /// @brief テキストカラーを取得
        /// @return テキストカラー
        [[nodiscard]] const Color& GetColor() const;

        //============================================================
        // Alignment
        //============================================================

        /// @brief 水平方向揃え位置を設定
        /// @param alignment 水平方向揃え位置
        void SetHorizontalAlignment(HorizontalAlignment alignment);

        /// @brief 水平方向揃え位置を取得
        /// @return 水平方向揃え位置
        [[nodiscard]] HorizontalAlignment GetHorizontalAlignment() const;

        /// @brief 垂直方向揃え位置を設定
        /// @param alignment 垂直方向揃え位置
        void SetVerticalAlignment(VerticalAlignment alignment);

        /// @brief 垂直方向揃え位置を取得
        /// @return 垂直方向揃え位置
        [[nodiscard]] VerticalAlignment GetVerticalAlignment() const;

    protected:
        std::wstring m_text;                                            ///< 表示テキスト
        std::wstring m_fontName = L"Segoe UI";                          ///< フォント名
        float m_fontSize = 16.0f;                                       ///< フォントサイズ
        Color m_color{ 1.0f, 1.0f, 1.0f, 1.0f };                       ///< テキストカラー
        HorizontalAlignment m_horizontalAlign = HorizontalAlignment::Left;  ///< 水平揃え
        VerticalAlignment m_verticalAlign = VerticalAlignment::Top;         ///< 垂直揃え
    };

} // namespace Engine
