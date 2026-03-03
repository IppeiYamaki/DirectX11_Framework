#pragma once

#include <Windows.h>
#include <string>

namespace Engine {

	/// @brief Application 起動設定
    /// @brief WinMain から渡す初期値をまとめるための構造体
	/// @note  設定の入れ物なので public メンバでOK（ルールに沿って m_ を付ける）
    struct ApplicationSettings final {
        // Win32
        HINSTANCE    m_hInstance        = nullptr;                  // インスタンスハンドル
        int          m_cmdShow          = SW_SHOW;                  // 表示方法

        // Window
        std::wstring m_title            = L"DirectX11_Framework";   // ウィンドウタイトル
        int          m_width            = 1280;                     // ウィンドウ幅
        int          m_height           = 720;                      // ウィンドウ高
        bool         m_isResizable      = true;                     // リサイズ可能か

        // Graphics
        bool         m_isVSyncEnabled   = true;                     // VSync 有効化

        // 必要になったら追加：
        // bool m_isFullscreen = false;
        // int m_backBufferCount = 2;

        bool IsValid() const {
            return (m_hInstance != nullptr) && (m_width > 0) && (m_height > 0);
        }
    };

} // namespace Engine
