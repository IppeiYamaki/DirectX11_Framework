#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "Engine/Graphics/ConstantBuffer.h"
#include "Engine/Graphics/ShaderConstants.h"

namespace Engine {

    class GraphicsDevice;
    class World;
    class Mesh;

    /**
     * @brief 1フレームの描画手順を統括するクラス
     *
     * - Clear →（描画）→ Present
     * - いまは最小検証用に DebugDraw を持つ
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
        // Debug draw (temporary)
        //============================================================
        void SetDebugDraw(
            Mesh* mesh,
            ID3D11InputLayout* inputLayout,
            ID3D11VertexShader* vertexShader,
            ID3D11PixelShader* pixelShader
        );

        void ClearDebugDraw();

        // 定数・リソースを外から差し替えたい時用（デフォルトは identity + 白）
        void SetDebugMatrices(const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection);
        void SetDebugMaterial(const MaterialParams& material);
        void SetDebugLight(const DirectionalLight& light);

        // Texture/Sampler（必要なら）
        void SetDebugTexture(ID3D11ShaderResourceView* srv);
        void SetDebugSampler(ID3D11SamplerState* sampler); // nullptrなら内部デフォルトを使う

    private:
        void BindDebugConstantsAndResources(ID3D11DeviceContext* context);


    private:
        GraphicsDevice*                 m_graphicsDevice        = nullptr; 
        bool                            m_isInitialized         = false;

		// Debug draw
        Mesh*                           m_debugMesh             = nullptr;  // メッシュは所有しない
        ID3D11InputLayout*              m_debugInputLayout      = nullptr;  // 入力レイアウトは所有しない
        ID3D11VertexShader*             m_debugVs               = nullptr;  // 頂点シェーダは所有しない
        ID3D11PixelShader*              m_debugPs               = nullptr;  // ピクセルシェーダは所有しない

        // Debug constant buffers
        ConstantBuffer<WorldCB>         m_worldCb;
        ConstantBuffer<ViewCB>          m_viewCb;
        ConstantBuffer<ProjectionCB>    m_projCb;
        ConstantBuffer<MaterialCB>      m_materialCb;
        ConstantBuffer<LightCB>         m_lightCb;

        // Debug constant data
        WorldCB                         m_worldData{};
        ViewCB                          m_viewData{};
        ProjectionCB                    m_projData{};
        MaterialCB                      m_materialData{};
        LightCB                         m_lightData{};

        // Debug texture/sampler (non-owning)
        ID3D11ShaderResourceView*       m_debugSrv              = nullptr;
        ID3D11SamplerState*             m_debugSamplerExternal  = nullptr;

        // Internal default sampler (owning)
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_defaultSampler;

    private:
        static constexpr float kDefaultClearColor[4] = { 0.10f, 0.10f, 0.18f, 1.0f };
    };

} // namespace Engine
