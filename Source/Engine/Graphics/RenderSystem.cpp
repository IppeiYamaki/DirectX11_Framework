#include "RenderSystem.h"

#include <algorithm>

#include "Engine/Core/Logger.h"
#include "Engine/Core/Assert.h"

#include "Engine/Graphics/GraphicsDevice.h"
#include "Engine/Graphics/Mesh.h"
#include "Engine/Graphics/Material.h"
#include "Engine/Scene/World.h"

namespace Engine {

    static bool CreateDepthStates(ID3D11Device* device,
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState>& dsDefault,
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState>& dsWriteOff,
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState>& dsOff) {
        // Default (Depth ON, Write ON)
        D3D11_DEPTH_STENCIL_DESC d{};
        d.DepthEnable = TRUE;
        d.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        d.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        d.StencilEnable = FALSE;

        if (FAILED(device->CreateDepthStencilState(&d, dsDefault.GetAddressOf()))) return false;

        // Depth ON, Write OFF
        d.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        if (FAILED(device->CreateDepthStencilState(&d, dsWriteOff.GetAddressOf()))) return false;

        // Depth OFF
        d.DepthEnable = FALSE;
        d.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        if (FAILED(device->CreateDepthStencilState(&d, dsOff.GetAddressOf()))) return false;

        return true;
    }

    static bool CreateRasterStates(ID3D11Device* device,
        Microsoft::WRL::ComPtr<ID3D11RasterizerState>& rsCullBack,
        Microsoft::WRL::ComPtr<ID3D11RasterizerState>& rsCullFront,
        Microsoft::WRL::ComPtr<ID3D11RasterizerState>& rsCullNone) {
        D3D11_RASTERIZER_DESC r{};
        r.FillMode = D3D11_FILL_SOLID;
        r.CullMode = D3D11_CULL_BACK;
        r.FrontCounterClockwise = FALSE;
        r.DepthClipEnable = TRUE;

        if (FAILED(device->CreateRasterizerState(&r, rsCullBack.GetAddressOf()))) return false;

        r.CullMode = D3D11_CULL_FRONT;
        if (FAILED(device->CreateRasterizerState(&r, rsCullFront.GetAddressOf()))) return false;

        r.CullMode = D3D11_CULL_NONE;
        if (FAILED(device->CreateRasterizerState(&r, rsCullNone.GetAddressOf()))) return false;

        return true;
    }

    static bool CreateBlendStates(ID3D11Device* device,
        Microsoft::WRL::ComPtr<ID3D11BlendState>& bsOpaque,
        Microsoft::WRL::ComPtr<ID3D11BlendState>& bsAlpha) {
        // Opaque
        D3D11_BLEND_DESC b{};
        b.AlphaToCoverageEnable = FALSE;
        b.IndependentBlendEnable = FALSE;

        auto& rt = b.RenderTarget[0];
        rt.BlendEnable = FALSE;
        rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        if (FAILED(device->CreateBlendState(&b, bsOpaque.GetAddressOf()))) return false;

        // Alpha blend
        rt.BlendEnable = TRUE;
        rt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        rt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        rt.BlendOp = D3D11_BLEND_OP_ADD;

        rt.SrcBlendAlpha = D3D11_BLEND_ONE;
        rt.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;

        if (FAILED(device->CreateBlendState(&b, bsAlpha.GetAddressOf()))) return false;

        return true;
    }

    bool RenderSystem::Initialize(GraphicsDevice& graphicsDevice) {
        if (m_isInitialized) return true;

        m_graphicsDevice = &graphicsDevice;

        ID3D11Device* device = m_graphicsDevice->GetDevice();
        if (!device) {
            Logger::Error("RenderSystem::Initialize failed: device is null.");
            Finalize();
            return false;
        }

        if (!m_worldCb.Create(device) || !m_viewCb.Create(device) || !m_projCb.Create(device) || !m_lightCb.Create(device)) {
            Logger::Error("RenderSystem::Initialize failed: create constant buffers failed.");
            Finalize();
            return false;
        }

        if (!CreateDepthStates(device, m_dsDefault, m_dsDepthWriteOff, m_dsDepthOff) ||
            !CreateRasterStates(device, m_rsCullBack, m_rsCullFront, m_rsCullNone) ||
            !CreateBlendStates(device, m_bsOpaque, m_bsAlpha)) {
            Logger::Error("RenderSystem::Initialize failed: create render states failed.");
            Finalize();
            return false;
        }

        DirectX::XMStoreFloat4x4(&m_viewData.g_view, DirectX::XMMatrixIdentity());
        DirectX::XMStoreFloat4x4(&m_projData.g_projection, DirectX::XMMatrixIdentity());

        m_lightData.g_light = DirectionalLight{};
        m_lightData.g_light.m_flags = 0;

        m_isInitialized = true;
        Logger::Info("RenderSystem initialized.");
        return true;
    }

    void RenderSystem::Finalize() {
        ClearRenderItems();

        m_worldCb.Destroy();
        m_viewCb.Destroy();
        m_projCb.Destroy();
        m_lightCb.Destroy();

        m_dsDefault.Reset();
        m_dsDepthWriteOff.Reset();
        m_dsDepthOff.Reset();

        m_rsCullBack.Reset();
        m_rsCullFront.Reset();
        m_rsCullNone.Reset();

        m_bsOpaque.Reset();
        m_bsAlpha.Reset();

        m_graphicsDevice = nullptr;
        m_isInitialized = false;
    }

    void RenderSystem::Reset() {
        ClearRenderItems();
    }

    bool RenderSystem::IsInitialized() const {
        return m_isInitialized;
    }

    void RenderSystem::AddRenderItem(const RenderItem& item) {
        if (!item.m_mesh || !item.m_material) return;
        m_items.push_back(item);
    }

    void RenderSystem::ClearRenderItems() {
        m_items.clear();
    }

    void RenderSystem::SetViewMatrix(const DirectX::XMFLOAT4X4& view) {
        m_viewData.g_view = view;
    }

    void RenderSystem::SetProjectionMatrix(const DirectX::XMFLOAT4X4& proj) {
        m_projData.g_projection = proj;
    }

    void RenderSystem::SetLight(const DirectionalLight& light) {
        m_lightData.g_light = light;
    }

    void RenderSystem::BindFrameConstants(ID3D11DeviceContext* ctx) {
        m_viewCb.Update(ctx, m_viewData);
        m_projCb.Update(ctx, m_projData);
        m_lightCb.Update(ctx, m_lightData);

        ID3D11Buffer* b1 = m_viewCb.GetBuffer();
        ID3D11Buffer* b2 = m_projCb.GetBuffer();
        ID3D11Buffer* b4 = m_lightCb.GetBuffer();

        ctx->VSSetConstantBuffers(1, 1, &b1);
        ctx->VSSetConstantBuffers(2, 1, &b2);
        ctx->VSSetConstantBuffers(4, 1, &b4);
        ctx->PSSetConstantBuffers(4, 1, &b4);
    }

    void RenderSystem::ApplyRenderStates(ID3D11DeviceContext* ctx, const RenderItem& item) {
        // Depth
        ID3D11DepthStencilState* ds = m_dsDefault.Get();
        if (item.m_stateFlags & kRenderStateDepthTestOff) {
            ds = m_dsDepthOff.Get();
        }
        else if (item.m_stateFlags & kRenderStateDepthWriteOff) {
            ds = m_dsDepthWriteOff.Get();
        }
        ctx->OMSetDepthStencilState(ds, 0);

        // Rasterizer
        ID3D11RasterizerState* rs = m_rsCullBack.Get();
        if (item.m_stateFlags & kRenderStateCullNone) {
            rs = m_rsCullNone.Get();
        }
        else if (item.m_stateFlags & kRenderStateCullFront) {
            rs = m_rsCullFront.Get();
        }
        ctx->RSSetState(rs);

        // Blend
        ID3D11BlendState* bs = m_bsOpaque.Get();
        if (item.m_stateFlags & kRenderStateBlendAlpha) {
            bs = m_bsAlpha.Get();
        }
        const float blendFactor[4] = { 0,0,0,0 };
        ctx->OMSetBlendState(bs, blendFactor, 0xFFFFFFFF);
    }

    void RenderSystem::DrawItem(ID3D11DeviceContext* ctx, const RenderItem& item) {
        ApplyRenderStates(ctx, item);

        // b0 (world)
        WorldCB w{};
        w.g_world = item.m_world;
        m_worldCb.Update(ctx, w);

        ID3D11Buffer* b0 = m_worldCb.GetBuffer();
        ctx->VSSetConstantBuffers(0, 1, &b0);

        ctx->IASetPrimitiveTopology(item.m_topology);

        item.m_mesh->Bind(ctx);
        item.m_material->Bind(ctx);
        item.m_mesh->Draw(ctx);
    }

    void RenderSystem::Draw(World& world) {
        ASSERT(m_isInitialized);
        ASSERT(m_graphicsDevice != nullptr);

        // フレーム開始時にRTV/DSVとビューポートをバインド
        m_graphicsDevice->BeginFrame();

        m_graphicsDevice->Clear(kDefaultClearColor);

        ID3D11DeviceContext* ctx = m_graphicsDevice->GetContext();
        ASSERT(ctx != nullptr);

        BindFrameConstants(ctx);

        // ��Layer/Order�Ń\�[�g�iUnity���ۂ��j
        std::stable_sort(m_items.begin(), m_items.end(),
            [](const RenderItem& a, const RenderItem& b) {
                if (a.m_layer != b.m_layer) {
                    return static_cast<int>(a.m_layer) < static_cast<int>(b.m_layer);
                }
                return a.m_orderInLayer < b.m_orderInLayer;
            });

        for (const auto& item : m_items) {
            if (!item.m_mesh || !item.m_mesh->IsValid()) continue;
            if (!item.m_material || !item.m_material->IsInitialized()) continue;
            DrawItem(ctx, item);
        }

        ClearRenderItems();

        (void)world;
        m_graphicsDevice->Present();
    }

} // namespace Engine
