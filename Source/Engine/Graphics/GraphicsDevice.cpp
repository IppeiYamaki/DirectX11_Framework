#include "GraphicsDevice.h"

#include "Engine/Core/Assert.h"
#include "Engine/Core/Logger.h"

#include <vector>

namespace Engine {

    GraphicsDevice::~GraphicsDevice() {
        Finalize();
    }

    bool GraphicsDevice::Initialize(HWND hwnd, int width, int height, bool isVSyncEnabled) {
        if (m_isInitialized) return true;

        if (hwnd == nullptr || width <= 0 || height <= 0) {
            Logger::Error("GraphicsDevice Initialize failed: invalid arguments.");
            return false;
        }

        m_hWnd = hwnd;
        m_width = width;
        m_height = height;
        m_isVSyncEnabled = isVSyncEnabled;

        UINT createFlags = 0;
#ifdef _DEBUG
        createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        // Feature level（まずは一般的な順で）
        const D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };

        DXGI_SWAP_CHAIN_DESC scDesc{};
        scDesc.BufferDesc.Width = static_cast<UINT>(m_width);
        scDesc.BufferDesc.Height = static_cast<UINT>(m_height);
        scDesc.BufferDesc.Format = kBackBufferFormat;
        scDesc.BufferDesc.RefreshRate.Numerator = 0;
        scDesc.BufferDesc.RefreshRate.Denominator = 0;
        scDesc.SampleDesc.Count = 1;
        scDesc.SampleDesc.Quality = 0;
        scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scDesc.BufferCount = 2; // 2が無難（後で設定化してもOK）
        scDesc.OutputWindow = m_hWnd;
        scDesc.Windowed = TRUE;
        scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // DX11最小構成（後でFLIPに移行してもOK）
        scDesc.Flags = 0;

        D3D_FEATURE_LEVEL createdLevel = D3D_FEATURE_LEVEL_11_0;

        HRESULT hr = ::D3D11CreateDeviceAndSwapChain(
            nullptr,                    // Adapter
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,                    // Software
            createFlags,
            featureLevels,
            static_cast<UINT>(_countof(featureLevels)),
            D3D11_SDK_VERSION,
            &scDesc,
            m_swapChain.GetAddressOf(),
            m_device.GetAddressOf(),
            &createdLevel,
            m_context.GetAddressOf()
        );

        if (FAILED(hr)) {
            // 11_1が無い環境などで落ちる場合があるので、最低限のフォールバックも用意
            Logger::Warn("D3D11CreateDeviceAndSwapChain failed. Retry with 11_0 only.");

            const D3D_FEATURE_LEVEL fallbackLevels[] = { D3D_FEATURE_LEVEL_11_0 };
            hr = ::D3D11CreateDeviceAndSwapChain(
                nullptr,
                D3D_DRIVER_TYPE_HARDWARE,
                nullptr,
                createFlags,
                fallbackLevels,
                static_cast<UINT>(_countof(fallbackLevels)),
                D3D11_SDK_VERSION,
                &scDesc,
                m_swapChain.GetAddressOf(),
                m_device.GetAddressOf(),
                &createdLevel,
                m_context.GetAddressOf()
            );
        }

        if (FAILED(hr)) {
            Engine::Assert::ReportHrFailure(hr, "D3D11CreateDeviceAndSwapChain", __FILE__, __LINE__, __func__);
            Finalize();
            return false;
        }

        if (!CreateBackBufferViews(m_width, m_height)) {
            Logger::Error("CreateBackBufferViews failed.");
            Finalize();
            return false;
        }

        BindDefaultTargetsAndViewport(m_width, m_height);

        m_isInitialized = true;
        Logger::Info("GraphicsDevice initialized.");
        return true;
    }

    void GraphicsDevice::Finalize() {
        if (m_context) {
            // リソース解放の前に状態を外す（安全策）
            m_context->OMSetRenderTargets(0, nullptr, nullptr);
            m_context->ClearState();
            m_context->Flush();
        }

        m_dsv.Reset();
        m_depthBuffer.Reset();
        m_rtv.Reset();

        m_swapChain.Reset();
        m_context.Reset();
        m_device.Reset();

        m_hWnd = nullptr;
        m_width = 0;
        m_height = 0;
        m_isVSyncEnabled = true;

        m_isInitialized = false;
    }

    void GraphicsDevice::Reset() {
        // 今は何もしない（必要になったら追加）
    }

    void GraphicsDevice::Clear(const float clearColor[4]) {
        if (!m_isInitialized) return;

        if (m_rtv) {
            m_context->ClearRenderTargetView(m_rtv.Get(), clearColor);
        }
        if (m_dsv) {
            m_context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        }
    }

    void GraphicsDevice::Present() {
        if (!m_isInitialized) return;

        const UINT syncInterval = m_isVSyncEnabled ? 1 : 0;
        const UINT flags = 0;
        m_swapChain->Present(syncInterval, flags);
    }

    bool GraphicsDevice::Resize(int width, int height) {
        if (!m_isInitialized) return false;
        if (width <= 0 || height <= 0) return false;

        if (width == m_width && height == m_height) {
            return true;
        }

        m_width = width;
        m_height = height;

        // バインド解除
        m_context->OMSetRenderTargets(0, nullptr, nullptr);

        // 既存ビュー破棄
        m_dsv.Reset();
        m_depthBuffer.Reset();
        m_rtv.Reset();

        // バッファサイズ変更
        HRESULT hr = m_swapChain->ResizeBuffers(
            0, // バッファ数は維持
            static_cast<UINT>(m_width),
            static_cast<UINT>(m_height),
            DXGI_FORMAT_UNKNOWN, // 既存フォーマット維持
            0
        );
        if (FAILED(hr)) {
            Engine::Assert::ReportHrFailure(hr, "IDXGISwapChain::ResizeBuffers", __FILE__, __LINE__, __func__);
            return false;
        }

        if (!CreateBackBufferViews(m_width, m_height)) {
            Logger::Error("CreateBackBufferViews failed on Resize.");
            return false;
        }

        BindDefaultTargetsAndViewport(m_width, m_height);
        return true;
    }

    bool GraphicsDevice::CreateBackBufferViews(int width, int height) {
        // BackBuffer → RTV
        Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
        HRESULT hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
        if (FAILED(hr)) {
            Engine::Assert::ReportHrFailure(hr, "IDXGISwapChain::GetBuffer", __FILE__, __LINE__, __func__);
            return false;
        }

        hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_rtv.GetAddressOf());
        if (FAILED(hr)) {
            Engine::Assert::ReportHrFailure(hr, "ID3D11Device::CreateRenderTargetView", __FILE__, __LINE__, __func__);
            return false;
        }

        // Depth → DSV
        D3D11_TEXTURE2D_DESC depthDesc{};
        depthDesc.Width = static_cast<UINT>(width);
        depthDesc.Height = static_cast<UINT>(height);
        depthDesc.MipLevels = 1;
        depthDesc.ArraySize = 1;
        depthDesc.Format = kDepthFormat;
        depthDesc.SampleDesc.Count = 1;
        depthDesc.SampleDesc.Quality = 0;
        depthDesc.Usage = D3D11_USAGE_DEFAULT;
        depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        hr = m_device->CreateTexture2D(&depthDesc, nullptr, m_depthBuffer.GetAddressOf());
        if (FAILED(hr)) {
            Engine::Assert::ReportHrFailure(hr, "ID3D11Device::CreateTexture2D (Depth)", __FILE__, __LINE__, __func__);
            return false;
        }

        hr = m_device->CreateDepthStencilView(m_depthBuffer.Get(), nullptr, m_dsv.GetAddressOf());
        if (FAILED(hr)) {
            Engine::Assert::ReportHrFailure(hr, "ID3D11Device::CreateDepthStencilView", __FILE__, __LINE__, __func__);
            return false;
        }

        return true;
    }

    void GraphicsDevice::BindDefaultTargetsAndViewport(int width, int height) {
        ID3D11RenderTargetView* rtvs[] = { m_rtv.Get() };
        m_context->OMSetRenderTargets(1, rtvs, m_dsv.Get());

        D3D11_VIEWPORT vp{};
        vp.TopLeftX = 0.0f;
        vp.TopLeftY = 0.0f;
        vp.Width = static_cast<float>(width);
        vp.Height = static_cast<float>(height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        m_context->RSSetViewports(1, &vp);
    }

    //============================================================
    // Getters
    //============================================================

    ID3D11Device* GraphicsDevice::GetDevice() const { return m_device.Get(); }
    ID3D11DeviceContext* GraphicsDevice::GetContext() const { return m_context.Get(); }
    IDXGISwapChain* GraphicsDevice::GetSwapChain() const { return m_swapChain.Get(); }

    ID3D11RenderTargetView* GraphicsDevice::GetRenderTargetView() const { return m_rtv.Get(); }
    ID3D11DepthStencilView* GraphicsDevice::GetDepthStencilView() const { return m_dsv.Get(); }

    int GraphicsDevice::GetWidth() const { return m_width; }
    int GraphicsDevice::GetHeight() const { return m_height; }

    bool GraphicsDevice::IsVSyncEnabled() const { return m_isVSyncEnabled; }
    bool GraphicsDevice::IsInitialized() const { return m_isInitialized; }

} // namespace Engine
