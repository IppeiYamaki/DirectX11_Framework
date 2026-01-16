#include "RenderSystem.h"

#include "Engine/Core/Logger.h"
#include "Engine/Core/Assert.h"

#include "Engine/Graphics/GraphicsDevice.h"
#include "Engine/Graphics/Mesh.h"
#include "Engine/Graphics/Material.h"
#include "Engine/Scene/World.h"

namespace Engine {

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

    void RenderSystem::DrawItem(ID3D11DeviceContext* ctx, const RenderItem& item) {
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

        m_graphicsDevice->Clear(kDefaultClearColor);

        ID3D11DeviceContext* ctx = m_graphicsDevice->GetContext();
        ASSERT(ctx != nullptr);

        BindFrameConstants(ctx);

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
