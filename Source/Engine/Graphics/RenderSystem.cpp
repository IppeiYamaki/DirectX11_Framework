#include "RenderSystem.h"

#include <algorithm>

#include "Engine/Core/Logger.h"
#include "Engine/Core/Assert.h"

#include "Engine/Graphics/GraphicsDevice.h"
#include "Engine/Graphics/Material.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Graphics/RenderTarget.h"
#include "Engine/Graphics/FadeSystem.h"
#include "Engine/Scene/World.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/LightSystem.h"
#include "Engine/UI/Canvas.h"
#include "Engine/Debug/DebugVisualizationSystem.h"
#include "Engine/Debug/DebugImGuiSystem.h"

namespace Engine {



    /// @brief UI頂点構造体
    struct UIVertex {
        float x, y;     // スクリーン座標
        float u, v;     // テクスチャ座標
    };

    /// @brief UI用定数バッファ
    struct UIConstantBuffer {
        float screenWidth;
        float screenHeight;
        float invScreenWidth;
        float invScreenHeight;
        float colorR;
        float colorG;
        float colorB;
        float colorA;
    };

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
        Microsoft::WRL::ComPtr<ID3D11BlendState>& bsAlpha,
        Microsoft::WRL::ComPtr<ID3D11BlendState>& bsAdditive) {
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

        // Additive blend (for particles/glow)
        rt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        rt.DestBlend = D3D11_BLEND_ONE;
        rt.BlendOp = D3D11_BLEND_OP_ADD;

        rt.SrcBlendAlpha = D3D11_BLEND_ONE;
        rt.DestBlendAlpha = D3D11_BLEND_ONE;
        rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;

        if (FAILED(device->CreateBlendState(&b, bsAdditive.GetAddressOf()))) return false;

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

        // 拡張ライト用定数バッファを作成
        if (!m_extendedLightCb.Create(device)) {
            Logger::Error("RenderSystem::Initialize failed: create extended light constant buffer failed.");
            Finalize();
            return false;
        }

        // クリップ平面用定数バッファを作成
        if (!m_clipPlaneCb.Create(device)) {
            Logger::Error("RenderSystem::Initialize failed: create clip plane constant buffer failed.");
            Finalize();
            return false;
        }

        if (!CreateDepthStates(device, m_dsDefault, m_dsDepthWriteOff, m_dsDepthOff) ||
            !CreateRasterStates(device, m_rsCullBack, m_rsCullFront, m_rsCullNone) ||
            !CreateBlendStates(device, m_bsOpaque, m_bsAlpha, m_bsAdditive)) {
            Logger::Error("RenderSystem::Initialize failed: create render states failed.");
            Finalize();
            return false;
        }

        DirectX::XMStoreFloat4x4(&m_viewData.g_view, DirectX::XMMatrixIdentity());
        DirectX::XMStoreFloat4x4(&m_projData.g_projection, DirectX::XMMatrixIdentity());

        m_lightData.g_light = DirectionalLight{};
        m_lightData.g_light.m_flags = 0;

        // 拡張ライトデータを初期化
        m_extendedLightData = ExtendedLightCB{};
        m_extendedLightingEnabled = false;

        // クリップ平面をデフォルト無効に
        m_clipPlaneEnabled = false;
        m_clipPlaneData.g_clipPlaneEnabled = 0.0f;

        m_isInitialized = true;
        Logger::Info("RenderSystem initialized.");
        return true;
    }

    void RenderSystem::Finalize() {
        ClearRenderItems();

        // UI resources
        FinalizeUIResources();

        m_worldCb.Destroy();
        m_viewCb.Destroy();
        m_projCb.Destroy();
        m_lightCb.Destroy();
        m_extendedLightCb.Destroy();
        m_clipPlaneCb.Destroy();

        m_dsDefault.Reset();
        m_dsDepthWriteOff.Reset();
        m_dsDepthOff.Reset();

        m_rsCullBack.Reset();
        m_rsCullFront.Reset();
        m_rsCullNone.Reset();

        m_bsOpaque.Reset();
        m_bsAlpha.Reset();
        m_bsAdditive.Reset();

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

    void RenderSystem::SetExtendedLights(const ExtendedLightCB& extendedLightData) {
        m_extendedLightData = extendedLightData;
    }

    void RenderSystem::SetExtendedLightingEnabled(bool enable) {
        m_extendedLightingEnabled = enable;
    }

    void RenderSystem::SetClipPlane(const DirectX::XMFLOAT4& plane) {
        m_clipPlaneData.g_clipPlane = plane;
        m_clipPlaneData.g_clipPlaneEnabled = 1.0f;
        m_clipPlaneEnabled = true;
    }

    void RenderSystem::ClearClipPlane() {
        m_clipPlaneData.g_clipPlaneEnabled = 0.0f;
        m_clipPlaneEnabled = false;
    }

    void RenderSystem::BindFrameConstants(ID3D11DeviceContext* ctx) {
        m_viewCb.Update(ctx, m_viewData);
        m_projCb.Update(ctx, m_projData);
        m_lightCb.Update(ctx, m_lightData);
        m_clipPlaneCb.Update(ctx, m_clipPlaneData);

        // 拡張ライトバッファを常に更新（シェーダーが参照するため）
        m_extendedLightCb.Update(ctx, m_extendedLightData);

        ID3D11Buffer* b1 = m_viewCb.GetBuffer();
        ID3D11Buffer* b2 = m_projCb.GetBuffer();
        ID3D11Buffer* b4 = m_lightCb.GetBuffer();
        ID3D11Buffer* b6 = m_clipPlaneCb.GetBuffer();
        ID3D11Buffer* b8 = m_extendedLightCb.GetBuffer();

        ctx->VSSetConstantBuffers(1, 1, &b1);
        ctx->VSSetConstantBuffers(2, 1, &b2);
        ctx->VSSetConstantBuffers(4, 1, &b4);
        ctx->VSSetConstantBuffers(6, 1, &b6);
        ctx->PSSetConstantBuffers(4, 1, &b4);

        // 拡張ライトバッファをピクセルシェーダーにバインド（b8）
        // 常にバインドしてシェーダーが参照できるようにする
        // Note: b7 is reserved for material-specific buffers (e.g., WaterBuffer)
        ctx->PSSetConstantBuffers(8, 1, &b8);
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
        if (item.m_stateFlags & kRenderStateBlendAdditive) {
            bs = m_bsAdditive.Get();
        }
        else if (item.m_stateFlags & kRenderStateBlendAlpha) {
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

    void RenderSystem::Draw(World& world, Canvas* canvas, FadeSystem* fadeSystem,
                            DebugVisualizationSystem* debugVisualization, Scene* scene, 
                            LightSystem* lightSystem, DebugImGuiSystem* debugImGui) {
        ASSERT(m_isInitialized);
        ASSERT(m_graphicsDevice != nullptr);

        // ImGuiフレーム開始（デバッグUIが有効な場合）
        if (debugImGui && debugImGui->IsEnabled()) {
            debugImGui->BeginFrame();
        }

        // フレーム開始時にRTV/DSVとビューポートをバインド
        m_graphicsDevice->BeginFrame();

        m_graphicsDevice->Clear(kDefaultClearColor);

        ID3D11DeviceContext* ctx = m_graphicsDevice->GetContext();
        ASSERT(ctx != nullptr);

        BindFrameConstants(ctx);

        // Layer/Orderでソート（Unity風）
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

        // デバッグ可視化描画（3Dオブジェクトの後、UIの前）
        // Debug Layerとして最後に描画され、深度テストOFFで常に前面に表示される
        if (debugVisualization && debugVisualization->IsEnabled()) {
            debugVisualization->Render(scene, lightSystem, m_viewData.g_view, m_projData.g_projection);
        }

        // UI描画（3Dの後、フェードの前）
        // DepthTest OFF + AlphaBlend ONはCanvas::Render内のDrawSpriteで設定される
        if (canvas && canvas->IsEnabled()) {
            Logger::Trace("RenderSystem::Draw: Rendering Canvas (UI) after 3D objects.");
            canvas->Render(this);
        }

        // フェード描画（UI描画の後、Present直前）
        // 最前面に描画して画面全体を覆う
        if (fadeSystem && fadeSystem->IsInitialized()) {
            Logger::Trace("RenderSystem::Draw: Rendering Fade overlay after Canvas.");
            fadeSystem->Draw();
        }

        // ImGuiデバッグUI描画（フェード描画の後、Present直前）
        // 開発者モード時のみ表示
        if (debugImGui && debugImGui->IsEnabled()) {
            debugImGui->Update(scene, lightSystem, debugVisualization);
            debugImGui->Render();
        }

        (void)world;
        m_graphicsDevice->Present();
    }

    //============================================================
    // UI Rendering Implementation
    //============================================================

    bool RenderSystem::InitializeUIResources() {
        if (m_uiResourcesInitialized) return true;
        if (!m_graphicsDevice) return false;

        ID3D11Device* device = m_graphicsDevice->GetDevice();
        if (!device) {
            Logger::Error("RenderSystem::InitializeUIResources failed: device is null.");
            return false;
        }

        // UI頂点シェーダーをロード
        m_uiVertexShader = std::make_unique<VertexShader>();
        if (!m_uiVertexShader->LoadFromCSO(device, L"Assets/Shaders/UIVS.cso")) {
            Logger::Error("RenderSystem::InitializeUIResources failed: could not load UIVS.cso");
            FinalizeUIResources();
            return false;
        }

        // UIピクセルシェーダーをロード
        m_uiPixelShader = std::make_unique<PixelShader>();
        if (!m_uiPixelShader->LoadFromCSO(device, L"Assets/Shaders/UIPS.cso")) {
            Logger::Error("RenderSystem::InitializeUIResources failed: could not load UIPS.cso");
            FinalizeUIResources();
            return false;
        }

        // UI用入力レイアウトを作成
        VertexInputLayout uiLayout;
        VertexInputElement posElement;
        posElement.m_semanticName = "POSITION";
        posElement.m_semanticIndex = 0;
        posElement.m_format = DXGI_FORMAT_R32G32_FLOAT;
        posElement.m_inputSlot = 0;
        posElement.m_alignedByteOffset = 0;
        posElement.m_inputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        posElement.m_instanceDataStepRate = 0;
        uiLayout.push_back(posElement);

        VertexInputElement uvElement;
        uvElement.m_semanticName = "TEXCOORD";
        uvElement.m_semanticIndex = 0;
        uvElement.m_format = DXGI_FORMAT_R32G32_FLOAT;
        uvElement.m_inputSlot = 0;
        uvElement.m_alignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        uvElement.m_inputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        uvElement.m_instanceDataStepRate = 0;
        uiLayout.push_back(uvElement);

        m_uiInputLayout = std::make_unique<InputLayout>();
        if (!m_uiInputLayout->Create(device, uiLayout,
            m_uiVertexShader->GetBytecodeData(),
            m_uiVertexShader->GetBytecodeSize())) {
            Logger::Error("RenderSystem::InitializeUIResources failed: could not create UI input layout");
            FinalizeUIResources();
            return false;
        }

        // 動的頂点バッファを作成（6頂点 = 2三角形）
        D3D11_BUFFER_DESC vbDesc{};
        vbDesc.ByteWidth = sizeof(UIVertex) * 6;
        vbDesc.Usage = D3D11_USAGE_DYNAMIC;
        vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        vbDesc.MiscFlags = 0;
        vbDesc.StructureByteStride = 0;

        HRESULT hr = device->CreateBuffer(&vbDesc, nullptr, m_uiVertexBuffer.GetAddressOf());
        if (FAILED(hr)) {
            Logger::Error("RenderSystem::InitializeUIResources failed: could not create UI vertex buffer");
            FinalizeUIResources();
            return false;
        }

        // UI定数バッファを作成
        D3D11_BUFFER_DESC cbDesc{};
        cbDesc.ByteWidth = sizeof(UIConstantBuffer);
        cbDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbDesc.MiscFlags = 0;
        cbDesc.StructureByteStride = 0;

        hr = device->CreateBuffer(&cbDesc, nullptr, m_uiConstantBuffer.GetAddressOf());
        if (FAILED(hr)) {
            Logger::Error("RenderSystem::InitializeUIResources failed: could not create UI constant buffer");
            FinalizeUIResources();
            return false;
        }

        // サンプラーステートを作成
        D3D11_SAMPLER_DESC samplerDesc{};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.MipLODBias = 0.0f;
        samplerDesc.MaxAnisotropy = 1;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.BorderColor[0] = 0.0f;
        samplerDesc.BorderColor[1] = 0.0f;
        samplerDesc.BorderColor[2] = 0.0f;
        samplerDesc.BorderColor[3] = 0.0f;
        samplerDesc.MinLOD = 0.0f;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

        hr = device->CreateSamplerState(&samplerDesc, m_uiSampler.GetAddressOf());
        if (FAILED(hr)) {
            Logger::Error("RenderSystem::InitializeUIResources failed: could not create UI sampler");
            FinalizeUIResources();
            return false;
        }

        // 1x1白テクスチャを作成（単色描画用ダミーテクスチャ）
        {
            D3D11_TEXTURE2D_DESC texDesc{};
            texDesc.Width = 1;
            texDesc.Height = 1;
            texDesc.MipLevels = 1;
            texDesc.ArraySize = 1;
            texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            texDesc.SampleDesc.Count = 1;
            texDesc.SampleDesc.Quality = 0;
            texDesc.Usage = D3D11_USAGE_IMMUTABLE;
            texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            texDesc.CPUAccessFlags = 0;
            texDesc.MiscFlags = 0;

            // 白色ピクセルデータ (RGBA: 255, 255, 255, 255)
            uint32_t whitePixel = 0xFFFFFFFF;
            D3D11_SUBRESOURCE_DATA initData{};
            initData.pSysMem = &whitePixel;
            initData.SysMemPitch = sizeof(uint32_t);
            initData.SysMemSlicePitch = 0;

            Microsoft::WRL::ComPtr<ID3D11Texture2D> whiteTex;
            hr = device->CreateTexture2D(&texDesc, &initData, whiteTex.GetAddressOf());
            if (FAILED(hr)) {
                Logger::Error("RenderSystem::InitializeUIResources failed: could not create white dummy texture");
                FinalizeUIResources();
                return false;
            }

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
            srvDesc.Format = texDesc.Format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = 1;

            hr = device->CreateShaderResourceView(whiteTex.Get(), &srvDesc, m_whiteDummySRV.GetAddressOf());
            if (FAILED(hr)) {
                Logger::Error("RenderSystem::InitializeUIResources failed: could not create white dummy SRV");
                FinalizeUIResources();
                return false;
            }
        }

        m_uiResourcesInitialized = true;
        Logger::Info("RenderSystem: UI resources initialized.");
        return true;
    }

    void RenderSystem::FinalizeUIResources() {
        m_whiteDummySRV.Reset();
        m_uiSampler.Reset();
        m_uiConstantBuffer.Reset();
        m_uiVertexBuffer.Reset();
        m_uiInputLayout.reset();
        m_uiPixelShader.reset();
        m_uiVertexShader.reset();
        m_uiResourcesInitialized = false;
    }

    void RenderSystem::DrawSprite(Texture* texture, float left, float top, float right, float bottom,
                                  float r, float g, float b, float a) {
        if (!m_isInitialized || !m_graphicsDevice || !texture || !texture->IsLoaded()) {
            Logger::Warn("RenderSystem::DrawSprite: Invalid state or texture not loaded.");
            return;
        }

        // UIリソースの遅延初期化
        if (!m_uiResourcesInitialized) {
            if (!InitializeUIResources()) {
                Logger::Error("RenderSystem::DrawSprite failed: could not initialize UI resources.");
                return;
            }
        }

        ID3D11DeviceContext* ctx = m_graphicsDevice->GetContext();
        if (!ctx) return;

        float screenW = static_cast<float>(m_graphicsDevice->GetWidth());
        float screenH = static_cast<float>(m_graphicsDevice->GetHeight());

        // 頂点データを更新（2三角形で四角形を構成）
        // 三角形1: 左上、右上、左下
        // 三角形2: 右上、右下、左下
        UIVertex vertices[6] = {
            { left,  top,    0.0f, 0.0f },  // 左上
            { right, top,    1.0f, 0.0f },  // 右上
            { left,  bottom, 0.0f, 1.0f },  // 左下
            { right, top,    1.0f, 0.0f },  // 右上
            { right, bottom, 1.0f, 1.0f },  // 右下
            { left,  bottom, 0.0f, 1.0f },  // 左下
        };

        // 頂点バッファを更新
        D3D11_MAPPED_SUBRESOURCE mapped;
        HRESULT hr = ctx->Map(m_uiVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (FAILED(hr)) {
            Logger::Error("RenderSystem::DrawSprite failed: could not map vertex buffer.");
            return;
        }
        memcpy(mapped.pData, vertices, sizeof(vertices));
        ctx->Unmap(m_uiVertexBuffer.Get(), 0);

        // 定数バッファを更新
        UIConstantBuffer cbData;
        cbData.screenWidth = screenW;
        cbData.screenHeight = screenH;
        cbData.invScreenWidth = 1.0f / screenW;
        cbData.invScreenHeight = 1.0f / screenH;
        cbData.colorR = r;
        cbData.colorG = g;
        cbData.colorB = b;
        cbData.colorA = a;

        hr = ctx->Map(m_uiConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (FAILED(hr)) {
            Logger::Error("RenderSystem::DrawSprite failed: could not map constant buffer.");
            return;
        }
        memcpy(mapped.pData, &cbData, sizeof(cbData));
        ctx->Unmap(m_uiConstantBuffer.Get(), 0);

        // 描画ステートを設定（深度テストOFF、アルファブレンドON）
        ctx->OMSetDepthStencilState(m_dsDepthOff.Get(), 0);
        ctx->RSSetState(m_rsCullNone.Get());
        const float blendFactor[4] = { 0, 0, 0, 0 };
        ctx->OMSetBlendState(m_bsAlpha.Get(), blendFactor, 0xFFFFFFFF);

        // シェーダーをバインド
        ctx->VSSetShader(m_uiVertexShader->GetShader(), nullptr, 0);
        ctx->PSSetShader(m_uiPixelShader->GetShader(), nullptr, 0);

        // 入力レイアウトをバインド
        ctx->IASetInputLayout(m_uiInputLayout->GetInputLayout());

        // 頂点バッファをバインド
        UINT stride = sizeof(UIVertex);
        UINT offset = 0;
        ID3D11Buffer* vb = m_uiVertexBuffer.Get();
        ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

        // 定数バッファをバインド
        ID3D11Buffer* cb = m_uiConstantBuffer.Get();
        ctx->VSSetConstantBuffers(0, 1, &cb);
        ctx->PSSetConstantBuffers(0, 1, &cb);

        // テクスチャとサンプラーをバインド
        ID3D11ShaderResourceView* srv = texture->GetShaderResourceView();
        ctx->PSSetShaderResources(0, 1, &srv);
        ID3D11SamplerState* sampler = m_uiSampler.Get();
        ctx->PSSetSamplers(0, 1, &sampler);

        // プリミティブトポロジーを設定
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // 描画
        ctx->Draw(6, 0);

        Logger::Trace("RenderSystem::DrawSprite: Drew sprite at (" + 
                      std::to_string(left) + ", " + std::to_string(top) + ") to (" +
                      std::to_string(right) + ", " + std::to_string(bottom) + ")");
    }

    void RenderSystem::DrawColoredRect(float left, float top, float right, float bottom,
                                       float r, float g, float b, float a) {
        if (!m_isInitialized || !m_graphicsDevice) {
            Logger::Warn("RenderSystem::DrawColoredRect: Invalid state.");
            return;
        }

        // アルファが0の場合は描画不要
        if (a <= 0.0001f) return;

        // UIリソースの遅延初期化
        if (!m_uiResourcesInitialized) {
            if (!InitializeUIResources()) {
                Logger::Error("RenderSystem::DrawColoredRect failed: could not initialize UI resources.");
                return;
            }
        }

        // 白ダミーテクスチャが必要
        if (!m_whiteDummySRV) {
            Logger::Error("RenderSystem::DrawColoredRect: White dummy SRV not available.");
            return;
        }

        ID3D11DeviceContext* ctx = m_graphicsDevice->GetContext();
        if (!ctx) return;

        float screenW = static_cast<float>(m_graphicsDevice->GetWidth());
        float screenH = static_cast<float>(m_graphicsDevice->GetHeight());

        // 頂点データを更新（2三角形で四角形を構成）
        UIVertex vertices[6] = {
            { left,  top,    0.0f, 0.0f },  // 左上
            { right, top,    1.0f, 0.0f },  // 右上
            { left,  bottom, 0.0f, 1.0f },  // 左下
            { right, top,    1.0f, 0.0f },  // 右上
            { right, bottom, 1.0f, 1.0f },  // 右下
            { left,  bottom, 0.0f, 1.0f },  // 左下
        };

        // 頂点バッファを更新
        D3D11_MAPPED_SUBRESOURCE mapped;
        HRESULT hr = ctx->Map(m_uiVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (FAILED(hr)) {
            Logger::Error("RenderSystem::DrawColoredRect failed: could not map vertex buffer.");
            return;
        }
        memcpy(mapped.pData, vertices, sizeof(vertices));
        ctx->Unmap(m_uiVertexBuffer.Get(), 0);

        // 定数バッファを更新
        UIConstantBuffer cbData;
        cbData.screenWidth = screenW;
        cbData.screenHeight = screenH;
        cbData.invScreenWidth = 1.0f / screenW;
        cbData.invScreenHeight = 1.0f / screenH;
        cbData.colorR = r;
        cbData.colorG = g;
        cbData.colorB = b;
        cbData.colorA = a;

        hr = ctx->Map(m_uiConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (FAILED(hr)) {
            Logger::Error("RenderSystem::DrawColoredRect failed: could not map constant buffer.");
            return;
        }
        memcpy(mapped.pData, &cbData, sizeof(cbData));
        ctx->Unmap(m_uiConstantBuffer.Get(), 0);

        // 描画ステートを設定（深度テストOFF、アルファブレンドON）
        ctx->OMSetDepthStencilState(m_dsDepthOff.Get(), 0);
        ctx->RSSetState(m_rsCullNone.Get());
        const float blendFactor[4] = { 0, 0, 0, 0 };
        ctx->OMSetBlendState(m_bsAlpha.Get(), blendFactor, 0xFFFFFFFF);

        // シェーダーをバインド
        ctx->VSSetShader(m_uiVertexShader->GetShader(), nullptr, 0);
        ctx->PSSetShader(m_uiPixelShader->GetShader(), nullptr, 0);

        // 入力レイアウトをバインド
        ctx->IASetInputLayout(m_uiInputLayout->GetInputLayout());

        // 頂点バッファをバインド
        UINT stride = sizeof(UIVertex);
        UINT offset = 0;
        ID3D11Buffer* vb = m_uiVertexBuffer.Get();
        ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

        // 定数バッファをバインド
        ID3D11Buffer* cb = m_uiConstantBuffer.Get();
        ctx->VSSetConstantBuffers(0, 1, &cb);
        ctx->PSSetConstantBuffers(0, 1, &cb);

        // 白ダミーテクスチャとサンプラーをバインド
        ID3D11ShaderResourceView* srv = m_whiteDummySRV.Get();
        ctx->PSSetShaderResources(0, 1, &srv);
        ID3D11SamplerState* sampler = m_uiSampler.Get();
        ctx->PSSetSamplers(0, 1, &sampler);

        // プリミティブトポロジーを設定
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // 描画
        ctx->Draw(6, 0);

        Logger::Trace("RenderSystem::DrawColoredRect: Drew colored rect at (" + 
                      std::to_string(left) + ", " + std::to_string(top) + ") to (" +
                      std::to_string(right) + ", " + std::to_string(bottom) + 
                      ") with alpha " + std::to_string(a));
    }

    //============================================================
    // Render Target Management
    //============================================================

    void RenderSystem::BeginRenderTarget(RenderTarget* target, const float* clearColor) {
        if (!target || !target->IsCreated()) {
            Logger::Error("RenderSystem::BeginRenderTarget: invalid target.");
            return;
        }

        ID3D11DeviceContext* ctx = m_graphicsDevice->GetContext();
        if (!ctx) return;

        // RTVとDSVをバインド
        ID3D11RenderTargetView* rtv = target->GetRenderTargetView();
        ID3D11DepthStencilView* dsv = target->GetDepthStencilView();
        ctx->OMSetRenderTargets(1, &rtv, dsv);

        // ビューポートを設定
        D3D11_VIEWPORT vp = target->GetViewport();
        ctx->RSSetViewports(1, &vp);

        // クリア
        if (clearColor) {
            ctx->ClearRenderTargetView(rtv, clearColor);
        }
        ctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        Logger::Trace("RenderSystem::BeginRenderTarget: Started rendering to target.");
    }

    void RenderSystem::EndRenderTarget() {
        // バックバッファに戻す
        if (m_graphicsDevice) {
            m_graphicsDevice->BeginFrame();
        }
        Logger::Trace("RenderSystem::EndRenderTarget: Returned to back buffer.");
    }

    void RenderSystem::FlushRenderItems(bool excludeReflectionItems) {
        ASSERT(m_isInitialized);
        ASSERT(m_graphicsDevice != nullptr);

        ID3D11DeviceContext* ctx = m_graphicsDevice->GetContext();
        ASSERT(ctx != nullptr);

        BindFrameConstants(ctx);

        // Layer/Orderでソート
        std::stable_sort(m_items.begin(), m_items.end(),
            [](const RenderItem& a, const RenderItem& b) {
                if (a.m_layer != b.m_layer) {
                    return static_cast<int>(a.m_layer) < static_cast<int>(b.m_layer);
                }
                return a.m_orderInLayer < b.m_orderInLayer;
            });

        for (const auto& item : m_items) {
            // 反射パスの場合、除外フラグが立っているアイテムをスキップ
            if (excludeReflectionItems && item.m_excludeFromReflection) {
                continue;
            }

            if (!item.m_mesh || !item.m_mesh->IsValid()) continue;
            if (!item.m_material || !item.m_material->IsInitialized()) continue;
            DrawItem(ctx, item);
        }

        // アイテムはクリアしない（メインパスでも使用するため）
        Logger::Trace("RenderSystem::FlushRenderItems: Rendered " + std::to_string(m_items.size()) + " items.");
    }

} // namespace Engine
