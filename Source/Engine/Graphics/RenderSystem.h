#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>

#include <vector>

#include "Engine/Graphics/ConstantBuffer.h"
#include "Engine/Graphics/ShaderConstants.h"

namespace Engine {

    class GraphicsDevice;
    class World;
    class Mesh;

    /**
     * @brief RenderSystem へ渡す描画要求（RenderQueueの1要素）
     *
     * - Mesh + (InputLayout/VS/PS)
     * - World(b0) と Material(b3) と Light(b4) を使う想定（Common.hlsl）
     * - Texture(t0)/Sampler(s0) も必要ならここで指定
     */
    struct RenderItem final {
        Mesh* m_mesh = nullptr;            // non-owning
        ID3D11InputLayout* m_inputLayout = nullptr;     // non-owning
        ID3D11VertexShader* m_vertexShader = nullptr;    // non-owning
        ID3D11PixelShader* m_pixelShader = nullptr;     // non-owning

        // Per-item constants
        DirectX::XMFLOAT4X4 m_world{};
        MaterialParams      m_material{};
        ID3D11ShaderResourceView* m_srv = nullptr;       // non-owning (t0)
        ID3D11SamplerState* m_sampler = nullptr;   // non-owning (s0). nullなら内部デフォルト

        D3D11_PRIMITIVE_TOPOLOGY m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        RenderItem() {
            DirectX::XMStoreFloat4x4(&m_world, DirectX::XMMatrixIdentity());
        }
    };

    /**
     * @brief 1フレームの描画手順を統括（Debug専用を廃止した版）
     *
     * - Game/World などから AddRenderItem で描画要求を積む
     * - Draw() で Clear → RenderQueue処理 → Present
     */

    /**
     * @brief 1フレームの描画手順を統括するクラス
     *
     * - Clear →（DebugDraw）→ Present
     * - DebugDraw 直前で World/View/Proj/Light を Update→Bind
     * - Material版 DebugDraw では、b3/t0/s0 は Material が Bind する
     */
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
        void DrawItem(ID3D11DeviceContext* context, const RenderItem& item);

    private:
        GraphicsDevice*                 m_graphicsDevice        = nullptr; 
        bool                            m_isInitialized         = false;

        // Queue
        std::vector<RenderItem>         m_renderItems;

        // Debug constant buffers
        ConstantBuffer<WorldCB>         m_worldCb;          // b0 (per item)
		ConstantBuffer<ViewCB>          m_viewCb;           // b1 (per frame)
        ConstantBuffer<ProjectionCB>    m_projCb;           // b2 (per frame)
        ConstantBuffer<MaterialCB>      m_materialCb;       // b3 (per item)
        ConstantBuffer<LightCB>         m_lightCb;          // b4 (per frame)

        // constant data
        ViewCB                          m_viewData{};
        ProjectionCB                    m_projData{};
        LightCB                         m_lightData{};

        // Internal default sampler (owning)
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_defaultSampler;

    private:
        static constexpr float kDefaultClearColor[4] = { 0.10f, 0.10f, 0.18f, 1.0f };
    };

} // namespace Engine
