/// @file   TextElement.cpp
/// @brief  テキスト表示用UI要素実装
#include "TextElement.h"

#include "Engine/Core/Logger.h"
#include "Engine/Graphics/RenderSystem.h"

namespace Engine {

    //============================================================
    // Lifecycle
    //============================================================

    void TextElement::Initialize() {
        UIElement::Initialize();
        // テキストレンダリングの初期化処理
        // 注: 実際のテキスト描画は DirectWrite や SpriteBatch を使用して実装する
        // 現段階ではプレースホルダーとして実装
    }

    void TextElement::Update(float deltaTime) {
        UIElement::Update(deltaTime);
        // テキスト要素の更新処理（必要に応じてアニメーションなど）
    }

    void TextElement::Render(RenderSystem* renderSystem) {
        if (!renderSystem || !m_isVisible) return;

        // 注: 実際のテキスト描画は DirectWrite + Direct2D や
        // SpriteBatch を使用して実装する必要がある
        // 現段階ではインターフェースのみ定義
        //
        // 将来の実装例:
        // 1. DirectWrite を使用してテキストレイアウトを作成
        // 2. Direct2D でレンダーターゲットに描画
        // 3. または SpriteFontを使用した描画

        (void)renderSystem;
    }

    void TextElement::OnDestroy() {
        UIElement::OnDestroy();
        m_text.clear();
    }

    //============================================================
    // Text
    //============================================================

    void TextElement::SetText(const std::wstring& text) {
        m_text = text;
    }

    const std::wstring& TextElement::GetText() const {
        return m_text;
    }

    //============================================================
    // Font Properties
    //============================================================

    void TextElement::SetFontSize(float fontSize) {
        m_fontSize = fontSize;
    }

    float TextElement::GetFontSize() const {
        return m_fontSize;
    }

    void TextElement::SetFontName(const std::wstring& fontName) {
        m_fontName = fontName;
    }

    const std::wstring& TextElement::GetFontName() const {
        return m_fontName;
    }

    //============================================================
    // Color
    //============================================================

    void TextElement::SetColor(const Color& color) {
        m_color = color;
    }

    const Color& TextElement::GetColor() const {
        return m_color;
    }

    //============================================================
    // Alignment
    //============================================================

    void TextElement::SetHorizontalAlignment(HorizontalAlignment alignment) {
        m_horizontalAlign = alignment;
    }

    TextElement::HorizontalAlignment TextElement::GetHorizontalAlignment() const {
        return m_horizontalAlign;
    }

    void TextElement::SetVerticalAlignment(VerticalAlignment alignment) {
        m_verticalAlign = alignment;
    }

    TextElement::VerticalAlignment TextElement::GetVerticalAlignment() const {
        return m_verticalAlign;
    }

} // namespace Engine
