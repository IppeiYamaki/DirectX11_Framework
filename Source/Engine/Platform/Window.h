#pragma once

#include <Windows.h>
#include <string>

namespace Engine {

    /// @brief Win32ウィンドウ管理クラス
	/// @note  Initialize() で生成、PumpMessages() を毎フレーム呼ぶ、Finalize() は安全に複数回呼べる
    class Window final {
    public:
        Window() = default;
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        //============================================================
        // Lifecycle
        //============================================================
        bool Initialize(
            HINSTANCE hInstance,
            int cmdShow,
            const std::wstring& title,
            int clientWidth,
            int clientHeight,
            bool isResizable
        );

        void Finalize();
        void Reset(); // 今は何もしない（将来の拡張用）

        //============================================================
        // Message pump
        //============================================================
        bool PumpMessages(); // false: 終了要求（WM_QUITなど）

        //============================================================
        // Getters (const)
        //============================================================
        HWND GetHwnd() const;
        int GetClientWidth() const;
        int GetClientHeight() const;

        bool IsInitialized() const;
        bool IsResizePending() const;

        /**
         * @brief リサイズ通知を消費する
         * @param outWidth 新しいクライアント幅
         * @param outHeight 新しいクライアント高さ
         * @return 直近でリサイズが発生していれば true
         */
        bool ConsumeResize(int& outWidth, int& outHeight);

    private:
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

        bool RegisterWindowClass();
        void UnregisterWindowClass();

    private:
		bool            m_isInitialized     = false;    /// 初期化済みフラグ

		HINSTANCE       m_hInstance         = nullptr;  /// インスタンスハンドル
		HWND            m_hWnd              = nullptr;  /// ウィンドウハンドル

		std::wstring    m_title{};                      /// ウィンドウタイトル
		bool            m_isResizable       = true;     /// リサイズ可能フラグ

		int             m_clientWidth       = 0;        /// クライアント幅
		int             m_clientHeight      = 0;        /// クライアント高さ

		bool            m_isResizePending   = false;    /// リサイズ保留フラグ
		int             m_pendingWidth      = 0;        /// リサイズ保留幅
		int             m_pendingHeight     = 0;        /// リサイズ保留高さ

    private:
        static constexpr const wchar_t* kWindowClassName = L"EngineWindowClass";
    };

} // namespace Engine
