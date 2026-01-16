#pragma once

#include <Windows.h>
#include <WindowsX.h> 

#include <array>
#include <cstdint>

namespace Engine {

    /**
     * @brief 入力管理クラス（キーボード + マウス）
     *
     * - Window の WndProc から HandleMessage() を呼んで状態を更新します
     * - 毎フレームの最初に BeginFrame() を呼び、"このフレームで押された/離された" をリセットします
     */
    class Input final {
    public:
        struct MessageResult {
            bool m_handled = false;
            LRESULT m_result = 0;
        };

        enum class MouseButton : std::uint8_t {
            Left = 0,
            Right = 1,
            Middle = 2,
            X1 = 3,
            X2 = 4,
        };

        //============================================================
        // Lifecycle
        //============================================================

        /// @brief 初期化（ウィンドウハンドルを保持）
        static void Initialize(HWND hwnd);
        static void Finalize();
        static void Reset();

        //============================================================
        // Frame
        //============================================================

        /// @brief 毎フレーム最初に呼ぶ（押した/離した/ホイール/移動量をリセット）
        static void BeginFrame();

        /// @brief Window::HandleMessage から呼ぶ（Win32 メッセージで状態更新）
        static MessageResult HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

        //============================================================
        // Keyboard
        //============================================================

        /// @brief キーが押されているか（押しっぱなし含む）
        static bool IsKeyPressed(int virtualKey);

        /// @brief このフレームで押されたか（押した瞬間）
        static bool IsKeyTriggered(int virtualKey);

        /// @brief このフレームで離されたか
        static bool IsKeyReleased(int virtualKey);

        //============================================================
        // Mouse
        //============================================================

        /// @brief マウス座標（クライアント座標）
        static POINT GetMousePosition();

        /// @brief マウス移動量（このフレーム中の合計 / クライアント座標）
        static POINT GetMouseDelta();

        /// @brief ホイール移動量（WHEEL_DELTA=120単位の合計）
        static int GetMouseWheelDelta();

        static bool IsMousePressed(MouseButton button);
        static bool IsMouseTriggered(MouseButton button);
        static bool IsMouseReleased(MouseButton button);

        //============================================================
        // Misc
        //============================================================
        static bool IsInitialized();

    private:
        static void ClearAllStates();

        static constexpr int kKeyCount = 256;
        static constexpr int kMouseButtonCount = 5;

        static bool s_isInitialized;
        static HWND s_hwnd;

        static std::array<bool, kKeyCount> s_keyDown;
        static std::array<bool, kKeyCount> s_keyPressed;
        static std::array<bool, kKeyCount> s_keyReleased;

        static std::array<bool, kMouseButtonCount> s_mouseDown;
        static std::array<bool, kMouseButtonCount> s_mousePressed;
        static std::array<bool, kMouseButtonCount> s_mouseReleased;

        static POINT s_mousePos;
        static POINT s_mouseDelta;
        static int s_wheelDelta;
    };

} // namespace Engine
