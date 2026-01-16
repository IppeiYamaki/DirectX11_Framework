#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>

#include <vector>
#include <cstdint>

#include "Engine/Graphics/ConstantBuffer.h"
#include "Engine/Graphics/ShaderConstants.h"
#include "Engine/Graphics/RenderLayer.h"

namespace Engine {

    class GraphicsDevice;
    class World;
    class Mesh;
    class Material;

    /**
     * @brief 描画要求（RenderQueueの1要素）
     */
    struct RenderItem final {
        Mesh* m_mesh = nullptr;      // non-owning
        Material* m_material = nullptr;  // non-owning
        DirectX::XMFLOAT4X4         m_world{};
        D3D11_PRIMITIVE_TOPOLOGY    m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        // ★Unityっぽい描画順
        RenderLayer                 m_layer = RenderLayer::Opaque;
        int                         m_orderInLayer = 0;

        // ★最低限の描画ステート
        std::uint32_t               m_stateFlags = kRenderStateNone;

        RenderItem() {
            DirectX::XMStoreFloat4x4(&m_world, DirectX::XMMatrixIdentity());
        }
    };

    class RenderSystem final {
    public:
        RenderSystem() = default;
        ~RenderSystem() = default;

        RenderSystem(const RenderSystem&) = delete;
        RenderSystem& operator=(const RenderSystem&) = delete;

        bool Initialize(GraphicsDevice& graphicsDevice);
        void Finalize();
        void Reset();

        void Draw(World& world);

        bool IsInitialized() const;

        //============================================================
        // RenderQueue
        //============================================================
        void AddRenderItem(const RenderItem& item);
        void ClearRenderItems();

        //============================================================
        // Frame constants (View/Projection/Light)
        //============================================================
        void SetViewMatrix(const DirectX::XMFLOAT4X4& view);
        void SetProjectionMatrix(const DirectX::XMFLOAT4X4& projection);
        void SetLight(const DirectionalLight& light);

    private:
        void BindFrameConstants(ID3D11DeviceContext* context); // b1,b2,b4
        void ApplyRenderStates(ID3D11DeviceContext* context, const RenderItem& item);
        void DrawItem(ID3D11DeviceContext* context, const RenderItem& item);

    private:
        GraphicsDevice* m_graphicsDevice = nullptr;
        bool m_isInitialized = false;

        std::vector<RenderItem> m_items;

        ConstantBuffer<WorldCB>         m_worldCb; // b0
        ConstantBuffer<ViewCB>          m_viewCb;  // b1
        ConstantBuffer<ProjectionCB>    m_projCb;  // b2
        ConstantBuffer<LightCB>         m_lightCb; // b4

        ViewCB       m_viewData{};
        ProjectionCB m_projData{};
        LightCB      m_lightData{};

        // ★RenderState（Skyなどのために最低限用意）
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_dsDefault;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_dsDepthWriteOff;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_dsDepthOff;

        Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rsCullBack;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rsCullFront;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rsCullNone;

        Microsoft::WRL::ComPtr<ID3D11BlendState> m_bsOpaque;
        Microsoft::WRL::ComPtr<ID3D11BlendState> m_bsAlpha;

    private:
        static constexpr float kDefaultClearColor[4] = { 0.10f, 0.10f, 0.18f, 1.0f };
    };

} // namespace Engine
