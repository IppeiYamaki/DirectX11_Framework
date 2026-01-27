#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

namespace Engine {

    /// @brief  D3D11のDevice/Context/SwapChainとRTV/DSVを管理する低レベル機能
    /// @note   Initializeで生成し、Clear/Present/Resizeを提供する
    class GraphicsDevice final {
    public:
        GraphicsDevice() = default;
        ~GraphicsDevice();

        GraphicsDevice(const GraphicsDevice&) = delete;
        GraphicsDevice& operator=(const GraphicsDevice&) = delete;

        //============================================================
        // Lifecycle
        //============================================================
        /// @brief  デバイスとスワップチェーンを初期化
        /// @param  hwnd Win32ウィンドウハンドル
        /// @param  width バックバッファ幅
        /// @param  height バックバッファ高さ
        /// @param  isVSyncEnabled VSync有効フラグ
        /// @return 成功ならtrue
        bool Initialize(HWND hwnd, int width, int height, bool isVSyncEnabled);
        /// @brief 終了処理
        void Finalize();
        /// @brief リセット（現在は何もしない）
        void Reset();

        //============================================================
        // Frame
        //============================================================
        /// @brief  画面をクリア
        /// @param  clearColor クリアカラーRGBA
        void Clear(const float clearColor[4]);
        /// @brief  バックバッファを表示
        void Present();

        //============================================================
        // Resize
        //============================================================
        /// @brief  バックバッファをリサイズ
        /// @param  width 新しい幅
        /// @param  height 新しい高さ
        /// @return 成功ならtrue
        bool Resize(int width, int height);

        //============================================================
        // Getters (borrow)
        //============================================================
        /// @brief  D3D11デバイスを取得
        /// @return D3D11デバイス（借用）
        ID3D11Device* GetDevice() const;
        /// @brief  D3D11コンテキストを取得
        /// @return D3D11デバイスコンテキスト（借用）
        ID3D11DeviceContext* GetContext() const;
        /// @brief  スワップチェーンを取得
        /// @return スワップチェーン（借用）
        IDXGISwapChain* GetSwapChain() const;

        /// @brief  レンダーターゲットビューを取得
        /// @return レンダーターゲットビュー（借用）
        ID3D11RenderTargetView* GetRenderTargetView() const;
        /// @brief  深度ステンシルビューを取得
        /// @return 深度ステンシルビュー（借用）
        ID3D11DepthStencilView* GetDepthStencilView() const;

        /// @brief  バックバッファ幅を取得
        /// @return バックバッファ幅
        int GetWidth() const;
        /// @brief  バックバッファ高さを取得
        /// @return バックバッファ高さ
        int GetHeight() const;

        /// @brief  VSync設定を取得
        /// @return VSync有効ならtrue
        bool IsVSyncEnabled() const;
        /// @brief  初期化済みか確認
        /// @return 初期化済みならtrue
        bool IsInitialized() const;

    private:
        /// @brief  バックバッファビューを生成
        /// @param  width バックバッファ幅
        /// @param  height バックバッファ高さ
        /// @return 成功ならtrue
        bool CreateBackBufferViews(int width, int height);
        /// @brief  既定のRTV/DSVとビューポートをバインド
        /// @param  width バックバッファ幅
        /// @param  height バックバッファ高さ
        void BindDefaultTargetsAndViewport(int width, int height);

    private:
        bool m_isInitialized = false;  ///< 初期化済みフラグ

        HWND m_hWnd = nullptr;         ///< ウィンドウハンドル
        int  m_width = 0;              ///< 幅
        int  m_height = 0;             ///< 高さ
        bool m_isVSyncEnabled = true;  ///< VSync有効フラグ

        Microsoft::WRL::ComPtr<ID3D11Device> m_device;          ///< D3D11デバイス
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;  ///< D3D11デバイスコンテキスト
        Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;     ///< スワップチェーン

        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;         ///< レンダーターゲットビュー
        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthBuffer;        ///< 深度バッファ
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv;         ///< 深度ステンシルビュー

    private:
        static constexpr DXGI_FORMAT kBackBufferFormat  = DXGI_FORMAT_R8G8B8A8_UNORM;
        static constexpr DXGI_FORMAT kDepthFormat       = DXGI_FORMAT_D24_UNORM_S8_UINT;
    };

} // namespace Engine
