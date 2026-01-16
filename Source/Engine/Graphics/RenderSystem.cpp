#include "RenderSystem.h"

#include "Engine/Core/Logger.h"
#include "Engine/Core/Assert.h"

#include "Engine/Graphics/GraphicsDevice.h"
#include "Engine/Graphics/Mesh.h"
#include "Engine/Scene/World.h"

namespace Engine {

    static bool CreateDefaultSampler(ID3D11Device* device, Microsoft::WRL::ComPtr<ID3D11SamplerState>& outSampler) {
        if (device == nullptr) return false;

        D3D11_SAMPLER_DESC desc{};
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        desc.MinLOD = 0;
        desc.MaxLOD = D3D11_FLOAT32_MAX;

        const HRESULT hr = device->CreateSamplerState(&desc, outSampler.GetAddressOf());
        if (FAILED(hr)) {
            Engine::Assert::ReportHrFailure(hr, "CreateSamplerState", __FILE__, __LINE__, __func__);
            outSampler.Reset();
            return false;
        }
        return true;
    }

    bool RenderSystem::Initialize(GraphicsDevice& graphicsDevice) {
        if (m_isInitialized) return true;

        m_graphicsDevice = &graphicsDevice;

        ID3D11Device* device = m_graphicsDevice->GetDevice();
        if (device == nullptr) {
            Logger::Error("RenderSystem::Initialize failed: device is null.");
            Finalize();
            return false;
        }

        if (!m_worldCb.Create(device) ||
            !m_viewCb.Create(device) ||
            !m_projCb.Create(device) ||
            !m_materialCb.Create(device) ||
            !m_lightCb.Create(device)) {
            Logger::Error("RenderSystem::Initialize failed: create constant buffers failed.");
            Finalize();
            return false;
        }

        if (!CreateDefaultSampler(device, m_defaultSampler)) {
            Logger::Error("RenderSystem::Initialize failed: create default sampler failed.");
            Finalize();
            return false;
        }

        // Default frame constants = identity, light off
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

        m_defaultSampler.Reset();

        m_worldCb.Destroy();
        m_viewCb.Destroy();
        m_projCb.Destroy();
        m_materialCb.Destroy();
        m_lightCb.Destroy();

        m_graphicsDevice = nullptr;
        m_isInitialized = false;
    }

    void RenderSystem::Reset() {
        ClearRenderItems();
    }

    bool RenderSystem::IsInitialized() const {
        return m_isInitialized;
    }

    //------------------------------------------------------------
    // RenderQueue
    //------------------------------------------------------------
    void RenderSystem::AddRenderItem(const RenderItem& item) {
        // 軽い検証：必要最低限が揃ってない場合は積まない
        if (item.m_mesh == nullptr ||
            item.m_inputLayout == nullptr ||
            item.m_vertexShader == nullptr ||
            item.m_pixelShader == nullptr) {
            return;
        }
        m_renderItems.push_back(item);
    }

    void RenderSystem::ClearRenderItems() {
        m_renderItems.clear();
    }

    //------------------------------------------------------------
    // Frame constants
    //------------------------------------------------------------
    void RenderSystem::SetViewMatrix(const DirectX::XMFLOAT4X4& view) {
        m_viewData.g_view = view;
    }

    void RenderSystem::SetProjectionMatrix(const DirectX::XMFLOAT4X4& projection) {
        m_projData.g_projection = projection;
    }

    void RenderSystem::SetLight(const DirectionalLight& light) {
        m_lightData.g_light = light;
    }

    void RenderSystem::BindFrameConstants(ID3D11DeviceContext* ctx) {
        // Update
        m_viewCb.Update(ctx, m_viewData);
        m_projCb.Update(ctx, m_projData);
        m_lightCb.Update(ctx, m_lightData);

        // Bind b1,b2,b4
        ID3D11Buffer* b1 = m_viewCb.GetBuffer();
        ID3D11Buffer* b2 = m_projCb.GetBuffer();
        ID3D11Buffer* b4 = m_lightCb.GetBuffer();

        ctx->VSSetConstantBuffers(1, 1, &b1);
        ctx->VSSetConstantBuffers(2, 1, &b2);
        ctx->VSSetConstantBuffers(4, 1, &b4);

        ctx->PSSetConstantBuffers(4, 1, &b4);
    }

    void RenderSystem::DrawItem(ID3D11DeviceContext* ctx, const RenderItem& item) {
        // Per-item: World (b0)
        WorldCB world{};
        world.g_world = item.m_world;
        m_worldCb.Update(ctx, world);
        ID3D11Buffer* b0 = m_worldCb.GetBuffer();
        ctx->VSSetConstantBuffers(0, 1, &b0);

        // Per-item: Material (b3)
        MaterialCB mat{};
        mat.g_material = item.m_material;
        m_materialCb.Update(ctx, mat);
        ID3D11Buffer* b3 = m_materialCb.GetBuffer();
        ctx->VSSetConstantBuffers(3, 1, &b3);
        ctx->PSSetConstantBuffers(3, 1, &b3);

        // Pipeline
        ctx->IASetInputLayout(item.m_inputLayout);
        ctx->IASetPrimitiveTopology(item.m_topology);

        item.m_mesh->Bind(ctx);

        ctx->VSSetShader(item.m_vertexShader, nullptr, 0);
        ctx->PSSetShader(item.m_pixelShader, nullptr, 0);

        // Texture/Sampler (t0/s0)
        ID3D11ShaderResourceView* srv = item.m_srv;
        ctx->PSSetShaderResources(0, 1, &srv);

        ID3D11SamplerState* sampler = item.m_sampler ? item.m_sampler : m_defaultSampler.Get();
        ctx->PSSetSamplers(0, 1, &sampler);

        item.m_mesh->Draw(ctx);
    }

    void RenderSystem::Draw(World& world) {
        ASSERT(m_isInitialized);
        ASSERT(m_graphicsDevice != nullptr);

        // 1) Clear
        m_graphicsDevice->Clear(kDefaultClearColor);

        ID3D11DeviceContext* ctx = m_graphicsDevice->GetContext();
        ASSERT(ctx != nullptr);

        // 2) Frame constants
        BindFrameConstants(ctx);

        // 3) RenderQueue
        for (const auto& item : m_renderItems) {
            if (item.m_mesh == nullptr || !item.m_mesh->IsValid()) continue;
            DrawItem(ctx, item);
        }

        // 4) Queue clear (次フレームは再提出)
        ClearRenderItems();

        (void)world;

        // 5) Present
        m_graphicsDevice->Present();
    }

} // namespace Engine
