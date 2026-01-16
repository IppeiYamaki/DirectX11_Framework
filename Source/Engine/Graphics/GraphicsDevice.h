#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

namespace Engine {

    /**
     * @brief D3D11 Device / Context / SwapChain と RTV/DSV を管理する低レベル層
     *
     * - Initialize() で生成
     * - Clear() / Present()
     * - Resize() でバックバッファ再生成（RTV/DSV/Viewport）
     */
    class GraphicsDevice final {
    public:
        GraphicsDevice() = default;
        ~GraphicsDevice();

        GraphicsDevice(const GraphicsDevice&) = delete;
        GraphicsDevice& operator=(const GraphicsDevice&) = delete;

        //============================================================
        // Lifecycle
        //============================================================
        bool Initialize(HWND hwnd, int width, int height, bool isVSyncEnabled);
        void Finalize();
        void Reset(); // 今は何もしない（将来拡張用）

        //============================================================
        // Frame
        //============================================================
        void Clear(const float clearColor[4]);
        void Present();

        //============================================================
        // Resize
        //============================================================
        bool Resize(int width, int height);

        //============================================================
        // Getters (borrow)
        //============================================================
        ID3D11Device* GetDevice() const;
        ID3D11DeviceContext* GetContext() const;
        IDXGISwapChain* GetSwapChain() const;

        ID3D11RenderTargetView* GetRenderTargetView() const;
        ID3D11DepthStencilView* GetDepthStencilView() const;

        int GetWidth() const;
        int GetHeight() const;

        bool IsVSyncEnabled() const;
        bool IsInitialized() const;

    private:
        bool CreateBackBufferViews(int width, int height);
        void BindDefaultTargetsAndViewport(int width, int height);

    private:
        bool m_isInitialized    = false;    // 初期化済みフラグ

        HWND m_hWnd             = nullptr;  // ウィンドウハンドル
        int  m_width            = 0;        // 幅
        int  m_height           = 0;        // 高さ
        bool m_isVSyncEnabled   = true;     // VSync有効フラグ

		Microsoft::WRL::ComPtr<ID3D11Device>            m_device;       // デバイス
		Microsoft::WRL::ComPtr<ID3D11DeviceContext>     m_context;      // デバイスコンテキスト
		Microsoft::WRL::ComPtr<IDXGISwapChain>          m_swapChain;    // スワップチェイン

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  m_rtv;          // レンダーターゲットビュー
		Microsoft::WRL::ComPtr<ID3D11Texture2D>         m_depthBuffer;  // デプスバッファ
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  m_dsv;          // デプスステンシルビュー

    private:
        static constexpr DXGI_FORMAT kBackBufferFormat  = DXGI_FORMAT_R8G8B8A8_UNORM;
        static constexpr DXGI_FORMAT kDepthFormat       = DXGI_FORMAT_D24_UNORM_S8_UINT;
    };

} // namespace Engine
