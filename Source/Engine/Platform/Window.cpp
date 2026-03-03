#include "Window.h"

#include <cassert>

#include "Engine/Core/Logger.h"
#include "Engine/Core/Assert.h"

#include "Engine/Platform/Input.h"
#include "Engine/Debug/DebugImGuiSystem.h"

namespace Engine {

    Window::~Window() {
        Finalize();
    }

    bool Window::Initialize(
        HINSTANCE hInstance,
        int cmdShow,
        const std::wstring& title,
        int clientWidth,
        int clientHeight,
        bool isResizable
    ) {
        if (m_isInitialized) return true;

        if (hInstance == nullptr || clientWidth <= 0 || clientHeight <= 0) {
            Logger::Error("Window Initialize failed: invalid arguments.");
            return false;
        }

        m_hInstance = hInstance;
        m_title = title;
        m_isResizable = isResizable;
        m_clientWidth = clientWidth;
        m_clientHeight = clientHeight;

        if (!RegisterWindowClass()) {
            Logger::Error("RegisterWindowClass failed.");
            Finalize();
            return false;
        }

        DWORD style = WS_OVERLAPPEDWINDOW;
        if (!m_isResizable) {
            // �T�C�Y�ύX�s�i�ő剻���g�̃T�C�Y�ύX�𖳌����j
            style &= ~WS_THICKFRAME;
            style &= ~WS_MAXIMIZEBOX;
        }

        RECT rect{ 0, 0, m_clientWidth, m_clientHeight };
        ::AdjustWindowRect(&rect, style, FALSE);

        const int windowWidth = rect.right - rect.left;
        const int windowHeight = rect.bottom - rect.top;

        m_hWnd = ::CreateWindowExW(
            0,
            kWindowClassName,
            m_title.c_str(),
            style,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            windowWidth,
            windowHeight,
            nullptr,
            nullptr,
            m_hInstance,
            this // �� WndProc �� this ���󂯎��iGWLP_USERDATA�ɕۑ��j
        );

        if (m_hWnd == nullptr) {
            Logger::Error("CreateWindowExW failed.");
            Finalize();
            return false;
        }

        ::ShowWindow(m_hWnd, cmdShow);
        ::UpdateWindow(m_hWnd);

        m_isResizePending = false;
        m_pendingWidth = m_clientWidth;
        m_pendingHeight = m_clientHeight;

        m_isInitialized = true;
        Logger::Info("Window initialized.");
        return true;
    }

    void Window::Finalize() {
        if (m_hWnd != nullptr) {
            ::DestroyWindow(m_hWnd);
            m_hWnd = nullptr;
        }

        UnregisterWindowClass();

        m_isInitialized = false;
        m_isResizePending = false;
        m_pendingWidth = 0;
        m_pendingHeight = 0;

        m_clientWidth = 0;
        m_clientHeight = 0;

        m_title.clear();
        m_hInstance = nullptr;
    }

    void Window::Reset() {
        // ���͉������Ȃ��i�K�v�ɂȂ�����ǉ��j
    }

    bool Window::PumpMessages() {
        MSG msg{};
        while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                return false;
            }
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        return true;
    }

    HWND Window::GetHwnd() const {
        return m_hWnd;
    }

    int Window::GetClientWidth() const {
        return m_clientWidth;
    }

    int Window::GetClientHeight() const {
        return m_clientHeight;
    }

    bool Window::IsInitialized() const {
        return m_isInitialized;
    }

    bool Window::IsResizePending() const {
        return m_isResizePending;
    }

    bool Window::ConsumeResize(int& outWidth, int& outHeight) {
        if (!m_isResizePending) return false;

        outWidth = m_pendingWidth;
        outHeight = m_pendingHeight;

        m_isResizePending = false;
        return true;
    }

    bool Window::RegisterWindowClass() {
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = &Window::WndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = m_hInstance;
        wc.hIcon = ::LoadIcon(nullptr, IDI_APPLICATION);
        wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpszMenuName = nullptr;
        wc.lpszClassName = kWindowClassName;
        wc.hIconSm = ::LoadIcon(nullptr, IDI_APPLICATION);

        const ATOM atom = ::RegisterClassExW(&wc);
        if (atom == 0) {
            return false;
        }
        return true;
    }

    void Window::UnregisterWindowClass() {
        if (m_hInstance == nullptr) return;

        // ���ɉ����ς݂ł����s���邾���Ȃ̂ŋC�ɂ��Ȃ����j
        ::UnregisterClassW(kWindowClassName, m_hInstance);
    }

    LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        Window* window = nullptr;

        if (msg == WM_NCCREATE) {
            auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
            window = reinterpret_cast<Window*>(cs->lpCreateParams);
            ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        }
        else {
            window = reinterpret_cast<Window*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        }

        if (window != nullptr) {
            return window->HandleMessage(hwnd, msg, wParam, lParam);
        }

        return ::DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    LRESULT Window::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        // ImGuiにメッセージを転送（ImGuiがメッセージを処理した場合は他に渡さない）
        if (DebugImGuiSystem::WndProcHandler(hwnd, msg, wParam, lParam)) {
            return 0;
        }

        // 次に Input へ中継（処理済みならここで終了）
        {
            const auto inputResult = Input::HandleMessage(hwnd, msg, wParam, lParam);
            if (inputResult.m_handled) {
                return inputResult.m_result;
            }
        }
        switch (msg) {
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;

        case WM_SIZE: {
            // クライアントサイズ更新（最小化時は 0 になることがある）
            const int w = LOWORD(lParam);
            const int h = HIWORD(lParam);

            m_clientWidth = w;
            m_clientHeight = h;

            // 0は無視（最小化など）
            if (w > 0 && h > 0) {
                m_pendingWidth = w;
                m_pendingHeight = h;
                m_isResizePending = true;
            }
            return 0;
        }

        default:
            break;
        }

        return ::DefWindowProcW(hwnd, msg, wParam, lParam);
    }

} // namespace Engine
