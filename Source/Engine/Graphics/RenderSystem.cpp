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

        // 定数バッファ作成
        if (!m_worldCb.Create(device) ||
            !m_viewCb.Create(device) ||
            !m_projCb.Create(device) ||
            !m_materialCb.Create(device) ||
            !m_lightCb.Create(device)) {
            Logger::Error("RenderSystem::Initialize failed: create constant buffers failed.");
            Finalize();
            return false;
        }

        // デフォルトSampler
        if (!CreateDefaultSampler(device, m_defaultSampler)) {
            Logger::Error("RenderSystem::Initialize failed: create default sampler failed.");
            Finalize();
            return false;
        }

        // デフォルト定数（identity + 白）
        m_worldData.g_world = MakeIdentityMatrix();
        m_viewData.g_view = MakeIdentityMatrix();
        m_projData.g_projection = MakeIdentityMatrix();

        m_materialData.g_material = MaterialParams{};
        m_materialData.g_material.m_baseColor = Vector4(1, 1, 1, 1);
        m_materialData.g_material.m_ambient = Vector4(1, 1, 1, 1);
        m_materialData.g_material.m_emissive = Vector4(0, 0, 0, 0);
        m_materialData.g_material.m_flags = 0; // テクスチャ使わない

        m_lightData.g_light = DirectionalLight{};
        m_lightData.g_light.m_flags = 0; // ライト無効（必要なら有効化）

        m_isInitialized = true;
        Logger::Info("RenderSystem initialized.");
        return true;
    }

    void RenderSystem::Finalize() {
        ClearDebugDraw();

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
        // 将来：RenderQueueクリア等
    }

    bool RenderSystem::IsInitialized() const {
        return m_isInitialized;
    }

    void RenderSystem::SetDebugDraw(
        Mesh* mesh,
        ID3D11InputLayout* inputLayout,
        ID3D11VertexShader* vertexShader,
        ID3D11PixelShader* pixelShader
    ) {
        m_debugMesh = mesh;
        m_debugInputLayout = inputLayout;
        m_debugVs = vertexShader;
        m_debugPs = pixelShader;
    }

    void RenderSystem::ClearDebugDraw() {
        m_debugMesh = nullptr;
        m_debugInputLayout = nullptr;
        m_debugVs = nullptr;
        m_debugPs = nullptr;

        m_debugSrv = nullptr;
        m_debugSamplerExternal = nullptr;
    }

    void RenderSystem::SetDebugMatrices(const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection) {
        m_worldData.g_world = world;
        m_viewData.g_view = view;
        m_projData.g_projection = projection;
    }

    void RenderSystem::SetDebugMaterial(const MaterialParams& material) {
        m_materialData.g_material = material;
    }

    void RenderSystem::SetDebugLight(const DirectionalLight& light) {
        m_lightData.g_light = light;
    }

    void RenderSystem::SetDebugTexture(ID3D11ShaderResourceView* srv) {
        m_debugSrv = srv;

        // srvがあるならフラグも立てる運用が便利（嫌ならGame側でフラグ管理してOK）
        if (srv) {
            m_materialData.g_material.m_flags |= kMaterialFlagUseTexture;
        }
        else {
            m_materialData.g_material.m_flags &= ~kMaterialFlagUseTexture;
        }
    }

    void RenderSystem::SetDebugSampler(ID3D11SamplerState* sampler) {
        m_debugSamplerExternal = sampler;
    }

    void RenderSystem::BindDebugConstantsAndResources(ID3D11DeviceContext* ctx) {
        // Update
        m_worldCb.Update(ctx, m_worldData);
        m_viewCb.Update(ctx, m_viewData);
        m_projCb.Update(ctx, m_projData);
        m_materialCb.Update(ctx, m_materialData);
        m_lightCb.Update(ctx, m_lightData);

        // Bind constant buffers (b0..b4)
        ID3D11Buffer* b0 = m_worldCb.GetBuffer();
        ID3D11Buffer* b1 = m_viewCb.GetBuffer();
        ID3D11Buffer* b2 = m_projCb.GetBuffer();
        ID3D11Buffer* b3 = m_materialCb.GetBuffer();
        ID3D11Buffer* b4 = m_lightCb.GetBuffer();

        // VS側：b0..b4（Lighting VS も想定して全部セット）
        ctx->VSSetConstantBuffers(0, 1, &b0);
        ctx->VSSetConstantBuffers(1, 1, &b1);
        ctx->VSSetConstantBuffers(2, 1, &b2);
        ctx->VSSetConstantBuffers(3, 1, &b3);
        ctx->VSSetConstantBuffers(4, 1, &b4);

        // PS側：主に Material/Light（b3,b4）
        ctx->PSSetConstantBuffers(3, 1, &b3);
        ctx->PSSetConstantBuffers(4, 1, &b4);

        // Texture/Sampler（t0 / s0）
        if (m_debugSrv) {
            ID3D11ShaderResourceView* srv = m_debugSrv;
            ctx->PSSetShaderResources(0, 1, &srv);
        }

        ID3D11SamplerState* sampler = m_debugSamplerExternal ? m_debugSamplerExternal : m_defaultSampler.Get();
        if (sampler) {
            ctx->PSSetSamplers(0, 1, &sampler);
        }
    }

    void RenderSystem::Draw(World& world) {
        ASSERT(m_isInitialized);
        ASSERT(m_graphicsDevice != nullptr);

        // 1) Clear
        m_graphicsDevice->Clear(kDefaultClearColor);

        ID3D11DeviceContext* ctx = m_graphicsDevice->GetContext();
        ASSERT(ctx != nullptr);

        // 2) Debug draw（最小：三角形1枚）
        if (m_debugMesh && m_debugMesh->IsValid() &&
            m_debugInputLayout && m_debugVs && m_debugPs) {

            // ★ここで定数/テクスチャを Bind（debug draw 直前）
            BindDebugConstantsAndResources(ctx);

            ctx->IASetInputLayout(m_debugInputLayout);
            ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            m_debugMesh->Bind(ctx);

            ctx->VSSetShader(m_debugVs, nullptr, 0);
            ctx->PSSetShader(m_debugPs, nullptr, 0);

            m_debugMesh->Draw(ctx);
        }

        (void)world;

        // 3) Present
        m_graphicsDevice->Present();
    }

} // namespace Engine
