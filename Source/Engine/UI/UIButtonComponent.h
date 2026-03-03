/// @file   UIButtonComponent.h
/// @brief  UIボタンコンポーネント（Unity風）
#pragma once

#include "Engine/Scene/Component.h"

#include <functional>

namespace Engine {

    /// @brief UIボタンコンポーネント
    /// @note  UIRectTransformの領域でクリック判定を行い、OnClickコールバックを発火
    class UIButtonComponent : public Component {
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
        UIButtonComponent() = default;
        ~UIButtonComponent() override = default;

        //============================================================
        // Interaction
        //============================================================

        /// @brief クリックイベントハンドラを設定
        /// @param callback クリック時に呼び出されるコールバック
        void SetOnClick(ClickCallback callback);

        /// @brief クリックイベントを発火
        void OnClick();

        /// @brief ボタンがクリック可能か設定
        /// @param interactable クリック可能ならtrue
        void SetInteractable(bool interactable);

        /// @brief ボタンがクリック可能か確認
        /// @return クリック可能ならtrue
        [[nodiscard]] bool IsInteractable() const;

        //============================================================
        // State
        //============================================================

        /// @brief 現在のボタン状態を取得
        /// @return ボタン状態
        [[nodiscard]] ButtonState GetButtonState() const;

        /// @brief ボタン状態を設定（Canvas側から呼び出し）
        /// @param state 設定するボタン状態
        void SetButtonState(ButtonState state);

        //============================================================
        // Input Handling
        //============================================================

        /// @brief マウス入力を処理
        /// @param isInside マウスがボタン領域内にあるか
        /// @param isPressed マウスボタンが押されているか
        /// @note  このメソッドはCanvas側から呼び出される
        void HandleMouseInput(bool isInside, bool isPressed);

        //============================================================
        // Lifecycle (Component overrides)
        //============================================================

        void OnDestroy() override;

    private:
        ClickCallback m_onClick;                            ///< クリックイベントハンドラ
        ButtonState m_state = ButtonState::Normal;          ///< 現在のボタン状態
        bool m_isInteractable = true;                       ///< クリック可能フラグ
        bool m_wasPressed = false;                          ///< 前フレームで押下されていたか
    };

} // namespace Engine
