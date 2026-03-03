#include "RenderTarget.h"

#include "Engine/Core/Logger.h"
#include "Engine/Core/Assert.h"

namespace Engine {

    RenderTarget::~RenderTarget() {
        Destroy();
    }

    bool RenderTarget::Create(ID3D11Device* device, std::uint32_t width, std::uint32_t height, DXGI_FORMAT format) {
        if (!device || width == 0 || height == 0) {
            Logger::Error("RenderTarget::Create failed: invalid arguments.");
            return false;
        }

        if (m_isCreated) {
            Destroy();
        }

        m_width = width;
        m_height = height;

        // カラーテクスチャを作成
        D3D11_TEXTURE2D_DESC texDesc{};
        texDesc.Width = width;
        texDesc.Height = height;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = format;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;

        HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, m_texture.GetAddressOf());
        if (FAILED(hr)) {
            Logger::Error("RenderTarget::Create failed: CreateTexture2D failed.");
            Destroy();
            return false;
        }

        // RTVを作成
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
        rtvDesc.Format = format;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;

        hr = device->CreateRenderTargetView(m_texture.Get(), &rtvDesc, m_rtv.GetAddressOf());
        if (FAILED(hr)) {
            Logger::Error("RenderTarget::Create failed: CreateRenderTargetView failed.");
            Destroy();
            return false;
        }

        // SRVを作成
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        hr = device->CreateShaderResourceView(m_texture.Get(), &srvDesc, m_srv.GetAddressOf());
        if (FAILED(hr)) {
            Logger::Error("RenderTarget::Create failed: CreateShaderResourceView failed.");
            Destroy();
            return false;
        }

        // 深度テクスチャを作成
        D3D11_TEXTURE2D_DESC depthDesc{};
        depthDesc.Width = width;
        depthDesc.Height = height;
        depthDesc.MipLevels = 1;
        depthDesc.ArraySize = 1;
        depthDesc.Format = kDepthFormat;
        depthDesc.SampleDesc.Count = 1;
        depthDesc.SampleDesc.Quality = 0;
        depthDesc.Usage = D3D11_USAGE_DEFAULT;
        depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthDesc.CPUAccessFlags = 0;
        depthDesc.MiscFlags = 0;

        hr = device->CreateTexture2D(&depthDesc, nullptr, m_depthTexture.GetAddressOf());
        if (FAILED(hr)) {
            Logger::Error("RenderTarget::Create failed: CreateTexture2D (depth) failed.");
            Destroy();
            return false;
        }

        // DSVを作成
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
        dsvDesc.Format = kDepthFormat;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;

        hr = device->CreateDepthStencilView(m_depthTexture.Get(), &dsvDesc, m_dsv.GetAddressOf());
        if (FAILED(hr)) {
            Logger::Error("RenderTarget::Create failed: CreateDepthStencilView failed.");
            Destroy();
            return false;
        }

        m_isCreated = true;
        Logger::Info("RenderTarget created: " + std::to_string(width) + "x" + std::to_string(height));
        return true;
    }

    void RenderTarget::Destroy() {
        m_dsv.Reset();
        m_depthTexture.Reset();
        m_srv.Reset();
        m_rtv.Reset();
        m_texture.Reset();

        m_width = 0;
        m_height = 0;
        m_isCreated = false;
    }

    bool RenderTarget::IsCreated() const {
        return m_isCreated;
    }

    ID3D11RenderTargetView* RenderTarget::GetRenderTargetView() const {
        return m_rtv.Get();
    }

    ID3D11DepthStencilView* RenderTarget::GetDepthStencilView() const {
        return m_dsv.Get();
    }

    ID3D11ShaderResourceView* RenderTarget::GetShaderResourceView() const {
        return m_srv.Get();
    }

    ID3D11Texture2D* RenderTarget::GetTexture() const {
        return m_texture.Get();
    }

    std::uint32_t RenderTarget::GetWidth() const {
        return m_width;
    }

    std::uint32_t RenderTarget::GetHeight() const {
        return m_height;
    }

    D3D11_VIEWPORT RenderTarget::GetViewport() const {
        D3D11_VIEWPORT vp{};
        vp.TopLeftX = 0.0f;
        vp.TopLeftY = 0.0f;
        vp.Width = static_cast<float>(m_width);
        vp.Height = static_cast<float>(m_height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        return vp;
    }

} // namespace Engine
