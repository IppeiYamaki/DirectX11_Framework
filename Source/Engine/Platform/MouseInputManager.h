/// @file   MouseInputManager.h
/// @brief  マウス入力イベントを管理するクラス
#pragma once

#include <functional>
#include <vector>

namespace Engine {

    /// @brief マウスのクリックイベントを管理するクラス
    /// @note  クリックリスナーを登録し、クリック時にイベントを通知する
    class MouseInputManager final {
    public:
        /// @brief クリックイベントリスナー型
        using ClickListener = std::function<void(float x, float y)>;

        MouseInputManager() = default;
        ~MouseInputManager() = default;

        MouseInputManager(const MouseInputManager&) = delete;
        MouseInputManager& operator=(const MouseInputManager&) = delete;

        //============================================================
        // Listener Registration
        //============================================================

        /// @brief クリックリスナーを登録
        /// @param listener クリック時に呼び出されるコールバック関数
        void RegisterClickListener(const ClickListener& listener);

        /// @brief 全てのクリックリスナーを削除
        void ClearClickListeners();

        /// @brief  登録されているリスナー数を取得
        /// @return リスナー数
        [[nodiscard]] std::size_t GetListenerCount() const;

        //============================================================
        // Frame Update
        //============================================================

        /// @brief 毎フレームの更新処理
        /// @note  マウスクリックを検出し、登録されたリスナーに通知する
        void Update();

    private:
        std::vector<ClickListener> m_clickListeners;    ///< 登録されているクリックリスナー
    };

} // namespace Engine
