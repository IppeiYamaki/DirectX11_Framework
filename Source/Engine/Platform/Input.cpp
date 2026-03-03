#include "Input.h"

#include <algorithm>

namespace Engine {

    bool Input::s_isInitialized = false;
    HWND Input::s_hwnd = nullptr;

    std::array<bool, Input::kKeyCount> Input::s_keyDown{};
    std::array<bool, Input::kKeyCount> Input::s_keyPressed{};
    std::array<bool, Input::kKeyCount> Input::s_keyReleased{};

    std::array<bool, Input::kMouseButtonCount> Input::s_mouseDown{};
    std::array<bool, Input::kMouseButtonCount> Input::s_mousePressed{};
    std::array<bool, Input::kMouseButtonCount> Input::s_mouseReleased{};

    POINT Input::s_mousePos{};
    POINT Input::s_mouseDelta{};
    int Input::s_wheelDelta = 0;

    //============================================================
    // Lifecycle
    //============================================================

    void Input::Initialize(HWND hwnd) {
        if (s_isInitialized) return;
        s_hwnd = hwnd;
        Reset();
        s_isInitialized = true;
    }

    void Input::Finalize() {
        if (!s_isInitialized) {
            s_hwnd = nullptr;
            return;
        }

        ClearAllStates();
        s_hwnd = nullptr;
        s_isInitialized = false;
    }

    void Input::Reset() {
        ClearAllStates();

        // �����̃}�E�X���W���擾�i�\�Ȃ�N���C�A���g���W�֕ϊ��j
        POINT p{};
        if (::GetCursorPos(&p) && s_hwnd != nullptr) {
            ::ScreenToClient(s_hwnd, &p);
        }
        s_mousePos = p;
    }

    //============================================================
    // Frame
    //============================================================

    void Input::BeginFrame() {
        if (!s_isInitialized) return;

        s_keyPressed.fill(false);
        s_keyReleased.fill(false);
        s_mousePressed.fill(false);
        s_mouseReleased.fill(false);

        s_mouseDelta = { 0, 0 };
        s_wheelDelta = 0;
    }

    //============================================================
    // Win32 message handling
    //============================================================

    Input::MessageResult Input::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (!s_isInitialized) {
            return {};
        }

        // hwnd �� Window ������n�������̂�D��i���S�j
        if (hwnd != nullptr) {
            s_hwnd = hwnd;
        }

        auto handleKeyDown = [&](int vk) {
            if (vk < 0 || vk >= kKeyCount) return;
            if (!s_keyDown[vk]) {
                s_keyPressed[vk] = true;
            }
            s_keyDown[vk] = true;
            };

        auto handleKeyUp = [&](int vk) {
            if (vk < 0 || vk >= kKeyCount) return;
            if (s_keyDown[vk]) {
                s_keyReleased[vk] = true;
            }
            s_keyDown[vk] = false;
            };

        auto handleMouseDown = [&](MouseButton btn) {
            const int idx = static_cast<int>(btn);
            if (idx < 0 || idx >= kMouseButtonCount) return;
            if (!s_mouseDown[idx]) {
                s_mousePressed[idx] = true;
            }
            s_mouseDown[idx] = true;
            };

        auto handleMouseUp = [&](MouseButton btn) {
            const int idx = static_cast<int>(btn);
            if (idx < 0 || idx >= kMouseButtonCount) return;
            if (s_mouseDown[idx]) {
                s_mouseReleased[idx] = true;
            }
            s_mouseDown[idx] = false;
            };

        switch (msg) {
            // --- Keyboard ---
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            handleKeyDown(static_cast<int>(wParam) & 0xFF);
            return { true, 0 };

        case WM_KEYUP:
        case WM_SYSKEYUP:
            handleKeyUp(static_cast<int>(wParam) & 0xFF);
            return { true, 0 };

            // �t�H�[�J�X���������牟�����ςȂ������Z�b�g�iAlt+Tab �Ȃǁj
        case WM_KILLFOCUS:
            ClearAllStates();
            return { false, 0 }; // Window ���̊��菈���ɔC���Ă�OK

            // --- Mouse move ---
        case WM_MOUSEMOVE: {
            const int x = GET_X_LPARAM(lParam);
            const int y = GET_Y_LPARAM(lParam);
            POINT newPos{ x, y };
            s_mouseDelta.x += (newPos.x - s_mousePos.x);
            s_mouseDelta.y += (newPos.y - s_mousePos.y);
            s_mousePos = newPos;
            return { true, 0 };
        }

                         // --- Mouse buttons ---
        case WM_LBUTTONDOWN:
            handleMouseDown(MouseButton::Left);
            return { true, 0 };
        case WM_LBUTTONUP:
            handleMouseUp(MouseButton::Left);
            return { true, 0 };

        case WM_RBUTTONDOWN:
            handleMouseDown(MouseButton::Right);
            return { true, 0 };
        case WM_RBUTTONUP:
            handleMouseUp(MouseButton::Right);
            return { true, 0 };

        case WM_MBUTTONDOWN:
            handleMouseDown(MouseButton::Middle);
            return { true, 0 };
        case WM_MBUTTONUP:
            handleMouseUp(MouseButton::Middle);
            return { true, 0 };

        case WM_XBUTTONDOWN: {
            const WORD xbtn = HIWORD(wParam);
            if (xbtn == XBUTTON1) handleMouseDown(MouseButton::X1);
            if (xbtn == XBUTTON2) handleMouseDown(MouseButton::X2);
            return { true, TRUE }; // WM_XBUTTON* �� TRUE ��Ԃ��̂�����
        }
        case WM_XBUTTONUP: {
            const WORD xbtn = HIWORD(wParam);
            if (xbtn == XBUTTON1) handleMouseUp(MouseButton::X1);
            if (xbtn == XBUTTON2) handleMouseUp(MouseButton::X2);
            return { true, TRUE };
        }

                         // --- Wheel ---
        case WM_MOUSEWHEEL: {
            s_wheelDelta += GET_WHEEL_DELTA_WPARAM(wParam);

            // lParam �̓X�N���[�����W�Ȃ̂ŃN���C�A���g��
            POINT p{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            if (s_hwnd != nullptr) {
                ::ScreenToClient(s_hwnd, &p);
            }
            s_mousePos = p;

            return { true, 0 };
        }

            // --- Window Size Change ---
            // ウィンドウサイズ変更時にマウス座標を再取得して正しい座標系に更新
        case WM_SIZE:
        case WM_EXITSIZEMOVE: {
            // 現在のマウス位置をスクリーン座標で取得し、新しいクライアント座標に変換
            POINT p{};
            if (::GetCursorPos(&p) && s_hwnd != nullptr && ::ScreenToClient(s_hwnd, &p)) {
                s_mousePos = p;
                // サイズ変更時はデルタをリセット（急激な変化を防ぐ）
                s_mouseDelta = { 0, 0 };
            }
            return { false, 0 }; // Window側の処理も継続させる
        }

        default:
            break;
        }

        return { false, 0 };
    }

    //============================================================
    // Keyboard
    //============================================================

    bool Input::IsKeyPressed(int virtualKey) {
        if (virtualKey < 0 || virtualKey >= kKeyCount) return false;
        return s_keyDown[virtualKey];
    }

    bool Input::IsKeyTriggered(int virtualKey) {
        if (virtualKey < 0 || virtualKey >= kKeyCount) return false;
        return s_keyPressed[virtualKey];
    }

    bool Input::IsKeyReleased(int virtualKey) {
        if (virtualKey < 0 || virtualKey >= kKeyCount) return false;
        return s_keyReleased[virtualKey];
    }

    //============================================================
    // Mouse
    //============================================================

    POINT Input::GetMousePosition() {
        return s_mousePos;
    }

    POINT Input::GetMouseDelta() {
        return s_mouseDelta;
    }

    int Input::GetMouseWheelDelta() {
        return s_wheelDelta;
    }

    bool Input::IsMousePressed(MouseButton button) {
        const int idx = static_cast<int>(button);
        if (idx < 0 || idx >= kMouseButtonCount) return false;
        return s_mouseDown[idx];
    }

    bool Input::IsMouseTriggered(MouseButton button) {
        const int idx = static_cast<int>(button);
        if (idx < 0 || idx >= kMouseButtonCount) return false;
        return s_mousePressed[idx];
    }

    bool Input::IsMouseReleased(MouseButton button) {
        const int idx = static_cast<int>(button);
        if (idx < 0 || idx >= kMouseButtonCount) return false;
        return s_mouseReleased[idx];
    }

    //============================================================
    // Misc
    //============================================================

    bool Input::IsInitialized() {
        return s_isInitialized;
    }

    void Input::ClearAllStates() {
        s_keyDown.fill(false);
        s_keyPressed.fill(false);
        s_keyReleased.fill(false);

        s_mouseDown.fill(false);
        s_mousePressed.fill(false);
        s_mouseReleased.fill(false);

        s_mouseDelta = { 0, 0 };
        s_wheelDelta = 0;
    }

} // namespace Engine
