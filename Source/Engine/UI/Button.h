/// @file   Button.h
/// @brief  ボタンUI要素
#pragma once

#include "Engine/UI/UIElement.h"
#include "Engine/Math/Color.h"

#include <functional>
#include <string>

namespace Engine {

    /// @brief ボタンUI要素
    /// @note  Canvas上にクリック可能なボタンを表示するためのUIコンポーネント
    class Button : public UIElement {
    public:
        /// @brief ボタンの状態
        enum class ButtonState {
            Normal,     ///< 通常状態
            Hovered,    ///< ホバー状態
            Pressed,    ///< 押下状態
            Disabled    ///< 無効状態
        };

        /// @brief クリックイベントコールバック型
        using ClickCallback = std::function<void()>;

    public:
        Button() = default;
        ~Button() override = default;

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

        /// @brief ボタンテキストを設定
        /// @param text 表示するテキスト
        void SetText(const std::wstring& text);

        /// @brief ボタンテキストを取得
        /// @return 表示テキスト
        [[nodiscard]] const std::wstring& GetText() const;

        /// @brief フォントサイズを設定
        /// @param fontSize フォントサイズ
        void SetFontSize(float fontSize);

        /// @brief フォントサイズを取得
        /// @return フォントサイズ
        [[nodiscard]] float GetFontSize() const;

        //============================================================
        // Colors
        //============================================================

        /// @brief 通常状態の背景色を設定
        /// @param color 背景色
        void SetNormalColor(const Color& color);

        /// @brief 通常状態の背景色を取得
        /// @return 背景色
        [[nodiscard]] const Color& GetNormalColor() const;

        /// @brief ホバー状態の背景色を設定
        /// @param color 背景色
        void SetHoveredColor(const Color& color);

        /// @brief ホバー状態の背景色を取得
        /// @return 背景色
        [[nodiscard]] const Color& GetHoveredColor() const;

        /// @brief 押下状態の背景色を設定
        /// @param color 背景色
        void SetPressedColor(const Color& color);

        /// @brief 押下状態の背景色を取得
        /// @return 背景色
        [[nodiscard]] const Color& GetPressedColor() const;

        /// @brief 無効状態の背景色を設定
        /// @param color 背景色
        void SetDisabledColor(const Color& color);

        /// @brief 無効状態の背景色を取得
        /// @return 背景色
        [[nodiscard]] const Color& GetDisabledColor() const;

        /// @brief テキストカラーを設定
        /// @param color テキストカラー
        void SetTextColor(const Color& color);

        /// @brief テキストカラーを取得
        /// @return テキストカラー
        [[nodiscard]] const Color& GetTextColor() const;

        //============================================================
        // Interaction
        //============================================================

        /// @brief クリックイベントハンドラを設定
        /// @param callback クリック時に呼び出されるコールバック
        void SetOnClick(ClickCallback callback);

        /// @brief ボタンがクリック可能か設定
        /// @param interactable クリック可能ならtrue
        void SetInteractable(bool interactable);

        /// @brief ボタンがクリック可能か確認
        /// @return クリック可能ならtrue
        [[nodiscard]] bool IsInteractable() const;

        /// @brief 現在のボタン状態を取得
        /// @return ボタン状態
        [[nodiscard]] ButtonState GetButtonState() const;

        //============================================================
        // Input Handling
        //============================================================

        /// @brief マウス位置がボタン領域内か確認
        /// @param mouseX マウスX座標
        /// @param mouseY マウスY座標
        /// @return 領域内ならtrue
        [[nodiscard]] bool ContainsPoint(float mouseX, float mouseY) const;

        /// @brief マウス入力を処理
        /// @param mouseX マウスX座標
        /// @param mouseY マウスY座標
        /// @param isPressed マウスボタンが押されているか
        void HandleMouseInput(float mouseX, float mouseY, bool isPressed);

    protected:
        /// @brief 現在のボタン状態に応じた背景色を取得
        /// @return 現在の背景色
        [[nodiscard]] const Color& GetCurrentBackgroundColor() const;

    protected:
        std::wstring m_text;                                        ///< ボタンテキスト
        float m_fontSize = 16.0f;                                   ///< フォントサイズ

        Color m_normalColor{ 0.3f, 0.3f, 0.3f, 1.0f };             ///< 通常状態の背景色
        Color m_hoveredColor{ 0.4f, 0.4f, 0.4f, 1.0f };            ///< ホバー状態の背景色
        Color m_pressedColor{ 0.2f, 0.2f, 0.2f, 1.0f };            ///< 押下状態の背景色
        Color m_disabledColor{ 0.2f, 0.2f, 0.2f, 0.5f };           ///< 無効状態の背景色
        Color m_textColor{ 1.0f, 1.0f, 1.0f, 1.0f };               ///< テキストカラー

        ButtonState m_state = ButtonState::Normal;                  ///< 現在のボタン状態
        bool m_isInteractable = true;                              ///< クリック可能フラグ
        bool m_wasPressed = false;                                 ///< 前フレームで押下されていたか

        ClickCallback m_onClick;                                    ///< クリックイベントハンドラ
    };

} // namespace Engine
