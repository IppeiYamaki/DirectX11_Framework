/// @file   DebugVisualizationSystem.cpp
/// @brief  デバッグ開発者モードの可視化システム実装
#include "DebugVisualizationSystem.h"

#include "Engine/Graphics/GraphicsDevice.h"
#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Graphics/Shader.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Graphics/Mesh.h"
#include "Engine/Graphics/MeshPrimitives.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/LightSystem.h"

#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/Components/MeshRenderer.h"
#include "Engine/Scene/Components/Transform.h"

#include "Engine/Debug/DebugLightMarkerPrefab.h"
#include "Game/Definitions/Prefabs/DebugLight.h"

#include "Engine/Platform/Input.h"
#include "Engine/Core/Logger.h"

#include <DirectXMath.h>

namespace Engine {

    /// @brief アウトライン用定数バッファ構造体
    struct OutlineConstantBuffer {
        DirectX::XMFLOAT4X4 world;
        DirectX::XMFLOAT4X4 view;
        DirectX::XMFLOAT4X4 projection;
        DirectX::XMFLOAT4 outlineColor;  // RGB + thickness
    };

    /// @brief ビルボード用定数バッファ構造体
    struct BillboardConstantBuffer {
        DirectX::XMFLOAT4X4 view;        // カメラ軸抽出用のビュー行列
        DirectX::XMFLOAT4X4 viewProj;    // 最終座標変換用のビュープロジェクション行列
        DirectX::XMFLOAT4 position;      // xyz = position, w = size
        DirectX::XMFLOAT4 color;         // RGBA
        DirectX::XMFLOAT4 direction;     // xyz = direction, w = show direction flag
    };

    /// @brief ビルボード頂点構造体
    struct BillboardVertex {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT2 texCoord;
    };

    /// @brief アンリット用定数バッファ構造体（3D球体表示用）
    struct UnlitConstantBuffer {
        DirectX::XMFLOAT4X4 world;       // ワールド行列
        DirectX::XMFLOAT4X4 view;        // ビュー行列
        DirectX::XMFLOAT4X4 projection;  // 射影行列
        DirectX::XMFLOAT4 color;         // RGBA color
    };

    DebugVisualizationSystem::~DebugVisualizationSystem() {
        Finalize();
    }

    bool DebugVisualizationSystem::Initialize(GraphicsDevice* graphicsDevice, RenderSystem* renderSystem) {
        if (m_isInitialized) return true;
        if (!graphicsDevice || !renderSystem) return false;

        m_graphicsDevice = graphicsDevice;
        m_renderSystem = renderSystem;

        auto* device = m_graphicsDevice->GetDevice();
        if (!device) return false;

        // アウトライン用リソースの初期化
        if (!InitializeOutlineResources()) {
            Logger::Error("DebugVisualizationSystem::Initialize failed: OutlineResources");
            return false;
        }

        // ビルボード用リソースの初期化
        if (!InitializeBillboardResources()) {
            Logger::Error("DebugVisualizationSystem::Initialize failed: BillboardResources");
            return false;
        }

        // ライトアイコンの読み込み
        if (!LoadLightIconTextures()) {
            Logger::Warn("DebugVisualizationSystem::Initialize: Light icon textures not loaded (will use default)");
            // アイコンがなくても動作する（警告のみ）
        }

        // ライト球体用リソースの初期化
        if (!InitializeLightSphereResources()) {
            Logger::Warn("DebugVisualizationSystem::Initialize: Light sphere resources not loaded (will use billboards only)");
            // 球体リソースがなくても動作する（警告のみ）
        }

        m_isInitialized = true;
        Logger::Info("DebugVisualizationSystem initialized");
        return true;
    }

    void DebugVisualizationSystem::Finalize() {
        if (!m_isInitialized) return;

        m_outlineVS.reset();
        m_outlinePS.reset();
        m_outlineInputLayout.reset();
        m_outlineConstantBuffer.Reset();

        m_billboardVS.reset();
        m_billboardPS.reset();
        m_billboardInputLayout.reset();
        m_billboardVertexBuffer.Reset();
        m_billboardConstantBuffer.Reset();

        m_arrowVertexBuffer.Reset();
        m_arrowConstantBuffer.Reset();

        m_pointLightIcon.reset();
        m_directionalLightIcon.reset();
        m_spotLightIcon.reset();

        // ライト球体用リソースの解放
        m_unlitVS.reset();
        m_unlitPS.reset();
        m_unlitInputLayout.reset();
        m_sphereMesh.reset();
        m_unlitConstantBuffer.Reset();

        m_rsCullFront.Reset();
        m_rsCullNone.Reset();
        m_bsAdditive.Reset();
        m_dsDepthOff.Reset();
        m_sampler.Reset();

        m_graphicsDevice = nullptr;
        m_renderSystem = nullptr;
        m_isInitialized = false;

        Logger::Info("DebugVisualizationSystem finalized");
    }

    void DebugVisualizationSystem::Update(float /*deltaTime*/, SceneContext& ctx, LightSystem* lightSystem) {
        if (!m_isInitialized) return;

        // 0キーでデバッグモードをトグル
        if (Input::IsKeyTriggered(kDebugModeToggleKey)) {
            ToggleEnabled();
            Logger::Info("Debug Visualization Mode: " + std::string(m_isEnabled ? "ON" : "OFF"));
        }
        
        // デバッグモードの状態変化を検出してライトマーカーを生成/削除
        if (m_isEnabled != m_wasEnabledLastFrame) {
            if (m_isEnabled) {
                // デバッグモード有効化時にライトマーカーを生成
                SpawnDebugLightMarkers(ctx, lightSystem);
            } else {
                // デバッグモード無効化時にライトマーカーを削除
                ClearDebugLightMarkers(ctx);
            }
        }
        
        // 次フレームのために現在の状態を保存
        m_wasEnabledLastFrame = m_isEnabled;
    }

    void DebugVisualizationSystem::Render(Scene* scene, LightSystem* lightSystem,
                                          const DirectX::XMFLOAT4X4& viewMatrix,
                                          const DirectX::XMFLOAT4X4& projMatrix) {
        if (!m_isInitialized || !m_isEnabled) return;

        int visType = static_cast<int>(m_visualizationType);

        // アウトライン描画
        if ((visType & static_cast<int>(DebugVisualizationType::Outline)) != 0) {
            if (scene) {
                RenderOutlines(scene, viewMatrix, projMatrix);
            }
        }

        // ライトアイコン描画（2Dビルボード）
        if ((visType & static_cast<int>(DebugVisualizationType::LightIcon)) != 0) {
            if (lightSystem) {
                RenderLightIcons(lightSystem, viewMatrix, projMatrix);
            }
        }

        // ライト位置に3D球体描画
        if ((visType & static_cast<int>(DebugVisualizationType::LightSphere)) != 0) {
            if (lightSystem && m_lightSphereEnabled) {
                RenderLightSpheres(lightSystem, viewMatrix, projMatrix);
            }
        }
    }

    void DebugVisualizationSystem::SetEnabled(bool enabled) {
        m_isEnabled = enabled;
    }

    bool DebugVisualizationSystem::IsEnabled() const {
        return m_isEnabled;
    }

    void DebugVisualizationSystem::ToggleEnabled() {
        m_isEnabled = !m_isEnabled;
    }

    void DebugVisualizationSystem::SetVisualizationType(DebugVisualizationType type) {
        m_visualizationType = type;
    }

    DebugVisualizationType DebugVisualizationSystem::GetVisualizationType() const {
        return m_visualizationType;
    }

    void DebugVisualizationSystem::SetOutlineColor(const DirectX::XMFLOAT3& color) {
        m_outlineColor = color;
    }

    DirectX::XMFLOAT3 DebugVisualizationSystem::GetOutlineColor() const {
        return m_outlineColor;
    }

    void DebugVisualizationSystem::SetOutlineThickness(float thickness) {
        m_outlineThickness = thickness;
    }

    float DebugVisualizationSystem::GetOutlineThickness() const {
        return m_outlineThickness;
    }

    void DebugVisualizationSystem::SetLightIconSize(float size) {
        m_lightIconSize = size;
    }

    float DebugVisualizationSystem::GetLightIconSize() const {
        return m_lightIconSize;
    }

    void DebugVisualizationSystem::SetDirectionalLightIconPosition(const DirectX::XMFLOAT3& position) {
        m_directionalLightIconPos = position;
    }

    DirectX::XMFLOAT3 DebugVisualizationSystem::GetDirectionalLightIconPosition() const {
        return m_directionalLightIconPos;
    }

    void DebugVisualizationSystem::SetLightSphereSize(float size) {
        m_lightSphereSize = size;
    }

    float DebugVisualizationSystem::GetLightSphereSize() const {
        return m_lightSphereSize;
    }

    void DebugVisualizationSystem::SetLightSphereEnabled(bool enabled) {
        m_lightSphereEnabled = enabled;
    }

    bool DebugVisualizationSystem::IsLightSphereEnabled() const {
        return m_lightSphereEnabled;
    }

    void DebugVisualizationSystem::SetDirectionalLightSpherePosition(const DirectX::XMFLOAT3& position) {
        m_directionalLightIconPos = position;
    }

    DirectX::XMFLOAT3 DebugVisualizationSystem::GetDirectionalLightSpherePosition() const {
        return m_directionalLightIconPos;
    }

    bool DebugVisualizationSystem::IsInitialized() const {
        return m_isInitialized;
    }

    bool DebugVisualizationSystem::InitializeOutlineResources() {
        auto* device = m_graphicsDevice->GetDevice();

        // アウトライン用頂点シェーダー
        m_outlineVS = std::make_unique<VertexShader>();
        if (!m_outlineVS->LoadFromCSO(device, L"Assets/Shaders/Debug/OutlineVS.cso")) {
            Logger::Error("Failed to load OutlineVS.cso");
            return false;
        }

        // アウトライン用ピクセルシェーダー
        m_outlinePS = std::make_unique<PixelShader>();
        if (!m_outlinePS->LoadFromCSO(device, L"Assets/Shaders/Debug/OutlinePS.cso")) {
            Logger::Error("Failed to load OutlinePS.cso");
            return false;
        }

        // 入力レイアウト（通常の3D頂点と同じ）
        VertexInputLayout layoutDesc;
        layoutDesc.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layoutDesc.push_back({ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layoutDesc.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 });

        m_outlineInputLayout = std::make_unique<InputLayout>();
        if (!m_outlineInputLayout->Create(device, layoutDesc, m_outlineVS->GetBytecodeData(), m_outlineVS->GetBytecodeSize())) {
            Logger::Error("Failed to create outline input layout");
            return false;
        }

        // 定数バッファ
        D3D11_BUFFER_DESC cbDesc{};
        cbDesc.ByteWidth = sizeof(OutlineConstantBuffer);
        cbDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        if (FAILED(device->CreateBuffer(&cbDesc, nullptr, m_outlineConstantBuffer.GetAddressOf()))) {
            Logger::Error("Failed to create outline constant buffer");
            return false;
        }

        // ラスタライザーステート（前面カリング - アウトライン用）
        D3D11_RASTERIZER_DESC rsDesc{};
        rsDesc.FillMode = D3D11_FILL_SOLID;
        rsDesc.CullMode = D3D11_CULL_FRONT;
        rsDesc.FrontCounterClockwise = FALSE;
        rsDesc.DepthClipEnable = TRUE;
        if (FAILED(device->CreateRasterizerState(&rsDesc, m_rsCullFront.GetAddressOf()))) {
            Logger::Error("Failed to create cull front rasterizer state");
            return false;
        }

        // カリングなし
        rsDesc.CullMode = D3D11_CULL_NONE;
        if (FAILED(device->CreateRasterizerState(&rsDesc, m_rsCullNone.GetAddressOf()))) {
            Logger::Error("Failed to create cull none rasterizer state");
            return false;
        }

        return true;
    }

    bool DebugVisualizationSystem::InitializeBillboardResources() {
        auto* device = m_graphicsDevice->GetDevice();

        // ビルボード用頂点シェーダー
        m_billboardVS = std::make_unique<VertexShader>();
        if (!m_billboardVS->LoadFromCSO(device, L"Assets/Shaders/Debug/BillboardVS.cso")) {
            Logger::Error("Failed to load BillboardVS.cso");
            return false;
        }

        // ビルボード用ピクセルシェーダー
        m_billboardPS = std::make_unique<PixelShader>();
        if (!m_billboardPS->LoadFromCSO(device, L"Assets/Shaders/Debug/BillboardPS.cso")) {
            Logger::Error("Failed to load BillboardPS.cso");
            return false;
        }

        // ビルボード入力レイアウト
        VertexInputLayout layoutDesc;
        layoutDesc.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layoutDesc.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 });

        m_billboardInputLayout = std::make_unique<InputLayout>();
        if (!m_billboardInputLayout->Create(device, layoutDesc, m_billboardVS->GetBytecodeData(), m_billboardVS->GetBytecodeSize())) {
            Logger::Error("Failed to create billboard input layout");
            return false;
        }

        // ビルボード頂点バッファを作成
        CreateBillboardVertexBuffer();

        // 定数バッファ
        D3D11_BUFFER_DESC cbDesc{};
        cbDesc.ByteWidth = sizeof(BillboardConstantBuffer);
        cbDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        if (FAILED(device->CreateBuffer(&cbDesc, nullptr, m_billboardConstantBuffer.GetAddressOf()))) {
            Logger::Error("Failed to create billboard constant buffer");
            return false;
        }

        // 加算合成ブレンドステート
        D3D11_BLEND_DESC bsDesc{};
        bsDesc.RenderTarget[0].BlendEnable = TRUE;
        bsDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        bsDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        bsDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        bsDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        bsDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        bsDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        bsDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        if (FAILED(device->CreateBlendState(&bsDesc, m_bsAdditive.GetAddressOf()))) {
            Logger::Error("Failed to create additive blend state");
            return false;
        }

        // 深度テストOFF
        D3D11_DEPTH_STENCIL_DESC dsDesc{};
        dsDesc.DepthEnable = FALSE;
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        dsDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
        if (FAILED(device->CreateDepthStencilState(&dsDesc, m_dsDepthOff.GetAddressOf()))) {
            Logger::Error("Failed to create depth off state");
            return false;
        }

        // サンプラーステート
        D3D11_SAMPLER_DESC sampDesc{};
        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
        if (FAILED(device->CreateSamplerState(&sampDesc, m_sampler.GetAddressOf()))) {
            Logger::Error("Failed to create sampler state");
            return false;
        }

        return true;
    }

    void DebugVisualizationSystem::CreateBillboardVertexBuffer() {
        auto* device = m_graphicsDevice->GetDevice();

        // ビルボードは4頂点のクアッド（2三角形）
        BillboardVertex vertices[] = {
            { {-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f} },
            { {-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f} },
            { { 0.5f, -0.5f, 0.0f}, {1.0f, 1.0f} },
            { { 0.5f,  0.5f, 0.0f}, {1.0f, 0.0f} },
        };

        D3D11_BUFFER_DESC vbDesc{};
        vbDesc.ByteWidth = sizeof(vertices);
        vbDesc.Usage = D3D11_USAGE_DEFAULT;
        vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = vertices;

        HRESULT hr = device->CreateBuffer(&vbDesc, &initData, m_billboardVertexBuffer.GetAddressOf());
        if (FAILED(hr)) {
            Logger::Error("Failed to create billboard vertex buffer");
        }
    }

    bool DebugVisualizationSystem::LoadLightIconTextures() {
        auto* device = m_graphicsDevice->GetDevice();

        // Note: テクスチャが見つからない場合でもshader側でフォールバック描画するため
        // 読み込み失敗は警告を出すのみで処理を継続する

        // PointLightアイコン
        m_pointLightIcon = std::make_unique<Texture>();
        if (!m_pointLightIcon->LoadFromFile(device, L"Assets/Editor/Debug/PointLightIcon.png")) {
            Logger::Warn("PointLightIcon.png not found");
        }

        // DirectionalLightアイコン
        m_directionalLightIcon = std::make_unique<Texture>();
        if (!m_directionalLightIcon->LoadFromFile(device, L"Assets/Editor/Debug/DirectionalLightIcon.png")) {
            Logger::Warn("DirectionalLightIcon.png not found");
        }

        // SpotLightアイコン
        m_spotLightIcon = std::make_unique<Texture>();
        if (!m_spotLightIcon->LoadFromFile(device, L"Assets/Editor/Debug/SpotLightIcon.png")) {
            Logger::Warn("SpotLightIcon.png not found");
        }

        // テクスチャがなくてもshader側のフォールバックで動作するためtrue返却
        return true;
    }

    bool DebugVisualizationSystem::InitializeLightSphereResources() {
        auto* device = m_graphicsDevice->GetDevice();

        // アンリット頂点シェーダー
        m_unlitVS = std::make_unique<VertexShader>();
        if (!m_unlitVS->LoadFromCSO(device, L"Assets/Shaders/Debug/UnlitVS.cso")) {
            Logger::Warn("UnlitVS.cso not found, light spheres will not be rendered");
            return false;
        }

        // アンリットピクセルシェーダー
        m_unlitPS = std::make_unique<PixelShader>();
        if (!m_unlitPS->LoadFromCSO(device, L"Assets/Shaders/Debug/UnlitPS.cso")) {
            Logger::Warn("UnlitPS.cso not found, light spheres will not be rendered");
            return false;
        }

        // 入力レイアウト（MeshPrimitivesの頂点構造体に合わせる）
        // Position(float3) + Normal(float3) + Color(float4) + UV(float2)
        VertexInputLayout layoutDesc;
        layoutDesc.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layoutDesc.push_back({ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layoutDesc.push_back({ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layoutDesc.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 });

        m_unlitInputLayout = std::make_unique<InputLayout>();
        if (!m_unlitInputLayout->Create(device, layoutDesc, m_unlitVS->GetBytecodeData(), m_unlitVS->GetBytecodeSize())) {
            Logger::Warn("Failed to create unlit input layout");
            return false;
        }

        // 定数バッファ
        D3D11_BUFFER_DESC cbDesc{};
        cbDesc.ByteWidth = sizeof(UnlitConstantBuffer);
        cbDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        if (FAILED(device->CreateBuffer(&cbDesc, nullptr, m_unlitConstantBuffer.GetAddressOf()))) {
            Logger::Warn("Failed to create unlit constant buffer");
            return false;
        }

        // 球体メッシュを生成
        m_sphereMesh = std::make_unique<Mesh>();
        if (!MeshPrimitives::CreateSphereUv(device, *m_sphereMesh, 1.0f, 16, 8)) {
            Logger::Warn("Failed to create sphere mesh for light visualization");
            return false;
        }

        Logger::Info("Light sphere resources initialized");
        return true;
    }

    void DebugVisualizationSystem::RenderOutlines(Scene* scene,
                                                   const DirectX::XMFLOAT4X4& viewMatrix,
                                                   const DirectX::XMFLOAT4X4& projMatrix) {
        if (!scene) return;

        auto* context = m_graphicsDevice->GetContext();

        // MeshRendererを持つ全GameObjectを取得
        auto objects = scene->FindObjectsWithComponent<MeshRenderer>();
        if (objects.empty()) return;

        // シェーダーとステートを設定
        context->VSSetShader(m_outlineVS->GetShader(), nullptr, 0);
        context->PSSetShader(m_outlinePS->GetShader(), nullptr, 0);
        context->IASetInputLayout(m_outlineInputLayout->GetInputLayout());
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // 前面カリング（背面を描画してアウトラインに）
        context->RSSetState(m_rsCullFront.Get());

        // 定数バッファを設定
        context->VSSetConstantBuffers(0, 1, m_outlineConstantBuffer.GetAddressOf());
        context->PSSetConstantBuffers(0, 1, m_outlineConstantBuffer.GetAddressOf());

        for (auto* obj : objects) {
            if (!obj || !obj->IsActive()) continue;

            auto* meshRenderer = obj->GetComponent<MeshRenderer>();
            if (!meshRenderer || !meshRenderer->IsEnabled()) continue;
            if (!meshRenderer->IsMeshReady()) continue;

            auto* transform = obj->GetTransform();
            if (!transform) continue;

            const Mesh* mesh = meshRenderer->GetMesh();
            if (!mesh || !mesh->IsValid()) continue;

            // ワールド行列を取得
            DirectX::XMFLOAT4X4 worldMatrix = transform->GetWorldMatrix();

            // 定数バッファを更新
            D3D11_MAPPED_SUBRESOURCE mapped{};
            if (SUCCEEDED(context->Map(m_outlineConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
                auto* cb = static_cast<OutlineConstantBuffer*>(mapped.pData);
                cb->world = worldMatrix;
                cb->view = viewMatrix;
                cb->projection = projMatrix;
                cb->outlineColor = { m_outlineColor.x, m_outlineColor.y, m_outlineColor.z, m_outlineThickness };
                context->Unmap(m_outlineConstantBuffer.Get(), 0);
            }

            // メッシュをバインドして描画
            mesh->Bind(context);
            mesh->Draw(context);
        }

        // ステートを戻す
        context->RSSetState(nullptr);
    }

    void DebugVisualizationSystem::RenderLightIcons(LightSystem* lightSystem,
                                                     const DirectX::XMFLOAT4X4& viewMatrix,
                                                     const DirectX::XMFLOAT4X4& projMatrix) {
        if (!lightSystem) return;

        auto* context = m_graphicsDevice->GetContext();

        // シェーダーとステートを設定
        context->VSSetShader(m_billboardVS->GetShader(), nullptr, 0);
        context->PSSetShader(m_billboardPS->GetShader(), nullptr, 0);
        context->IASetInputLayout(m_billboardInputLayout->GetInputLayout());
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        // 頂点バッファを設定
        UINT stride = sizeof(BillboardVertex);
        UINT offset = 0;
        context->IASetVertexBuffers(0, 1, m_billboardVertexBuffer.GetAddressOf(), &stride, &offset);

        // ブレンドと深度ステートを設定
        const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        context->OMSetBlendState(m_bsAdditive.Get(), blendFactor, 0xFFFFFFFF);
        context->OMSetDepthStencilState(m_dsDepthOff.Get(), 0);

        // 定数バッファとサンプラーを設定
        context->VSSetConstantBuffers(0, 1, m_billboardConstantBuffer.GetAddressOf());
        context->PSSetConstantBuffers(0, 1, m_billboardConstantBuffer.GetAddressOf());
        context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());

        // View * Projection行列を計算
        DirectX::XMMATRIX viewMat = DirectX::XMLoadFloat4x4(&viewMatrix);
        DirectX::XMMATRIX projMat = DirectX::XMLoadFloat4x4(&projMatrix);
        DirectX::XMMATRIX viewProj = viewMat * projMat;
        DirectX::XMFLOAT4X4 viewProjMatrix;
        DirectX::XMStoreFloat4x4(&viewProjMatrix, viewProj);

        // Point Lights
        auto pointLights = lightSystem->GetPointLights();
        for (auto* light : pointLights) {
            if (!light || !light->IsEnabled()) continue;

            Vector3 pos = light->GetPosition();
            Vector3 color = light->GetColor();

            // 定数バッファを更新
            D3D11_MAPPED_SUBRESOURCE mapped{};
            if (SUCCEEDED(context->Map(m_billboardConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
                auto* cb = static_cast<BillboardConstantBuffer*>(mapped.pData);
                cb->view = viewMatrix;
                cb->viewProj = viewProjMatrix;
                cb->position = { pos.x, pos.y, pos.z, m_lightIconSize };
                cb->color = { color.x, color.y, color.z, 1.0f };
                cb->direction = { 0.0f, 0.0f, 0.0f, 0.0f };  // 方向表示なし
                context->Unmap(m_billboardConstantBuffer.Get(), 0);
            }

            // テクスチャを設定
            if (m_pointLightIcon && m_pointLightIcon->IsLoaded()) {
                auto* srv = m_pointLightIcon->GetShaderResourceView();
                context->PSSetShaderResources(0, 1, &srv);
            }

            // 描画
            context->Draw(4, 0);
        }

        // Directional Lights
        auto directionalLights = lightSystem->GetDirectionalLights();
        for (auto* light : directionalLights) {
            if (!light || !light->IsEnabled()) continue;

            Vector3 dir = light->GetDirection();
            Vector3 color = light->GetColor();

            // DirectionalLightは位置がないため、設定された表示位置に表示
            // m_directionalLightIconPosで位置をカスタマイズ可能
            const auto& displayPos = m_directionalLightIconPos;

            // 定数バッファを更新
            D3D11_MAPPED_SUBRESOURCE mapped{};
            if (SUCCEEDED(context->Map(m_billboardConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
                auto* cb = static_cast<BillboardConstantBuffer*>(mapped.pData);
                cb->view = viewMatrix;
                cb->viewProj = viewProjMatrix;
                cb->position = { displayPos.x, displayPos.y, displayPos.z, m_lightIconSize * 1.5f };
                cb->color = { color.x, color.y, color.z, 1.0f };
                cb->direction = { dir.x, dir.y, dir.z, 1.0f };  // 方向表示あり
                context->Unmap(m_billboardConstantBuffer.Get(), 0);
            }

            // テクスチャを設定
            if (m_directionalLightIcon && m_directionalLightIcon->IsLoaded()) {
                auto* srv = m_directionalLightIcon->GetShaderResourceView();
                context->PSSetShaderResources(0, 1, &srv);
            }

            // 描画
            context->Draw(4, 0);
        }

        // Spot Lights
        auto spotLights = lightSystem->GetSpotLights();
        for (auto* light : spotLights) {
            if (!light || !light->IsEnabled()) continue;

            Vector3 pos = light->GetPosition();
            Vector3 dir = light->GetDirection();
            Vector3 color = light->GetColor();

            // 定数バッファを更新
            D3D11_MAPPED_SUBRESOURCE mapped{};
            if (SUCCEEDED(context->Map(m_billboardConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
                auto* cb = static_cast<BillboardConstantBuffer*>(mapped.pData);
                cb->view = viewMatrix;
                cb->viewProj = viewProjMatrix;
                cb->position = { pos.x, pos.y, pos.z, m_lightIconSize };
                cb->color = { color.x, color.y, color.z, 1.0f };
                cb->direction = { dir.x, dir.y, dir.z, 1.0f };  // 方向表示あり
                context->Unmap(m_billboardConstantBuffer.Get(), 0);
            }

            // テクスチャを設定
            if (m_spotLightIcon && m_spotLightIcon->IsLoaded()) {
                auto* srv = m_spotLightIcon->GetShaderResourceView();
                context->PSSetShaderResources(0, 1, &srv);
            }

            // 描画
            context->Draw(4, 0);
        }

        // ステートをリセット
        context->OMSetBlendState(nullptr, blendFactor, 0xFFFFFFFF);
        context->OMSetDepthStencilState(nullptr, 0);
    }

    void DebugVisualizationSystem::RenderLightSpheres(LightSystem* lightSystem,
                                                       const DirectX::XMFLOAT4X4& viewMatrix,
                                                       const DirectX::XMFLOAT4X4& projMatrix) {
        if (!lightSystem) return;
        if (!m_unlitVS || !m_unlitPS || !m_sphereMesh || !m_sphereMesh->IsValid()) return;

        auto* context = m_graphicsDevice->GetContext();

        // シェーダーとステートを設定
        context->VSSetShader(m_unlitVS->GetShader(), nullptr, 0);
        context->PSSetShader(m_unlitPS->GetShader(), nullptr, 0);
        context->IASetInputLayout(m_unlitInputLayout->GetInputLayout());
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // 定数バッファを設定
        context->VSSetConstantBuffers(0, 1, m_unlitConstantBuffer.GetAddressOf());
        context->PSSetConstantBuffers(0, 1, m_unlitConstantBuffer.GetAddressOf());

        // 通常カリング（背面カリング）
        context->RSSetState(nullptr);

        // Point Lights（ライト色の球体）
        auto pointLights = lightSystem->GetPointLights();
        for (auto* light : pointLights) {
            if (!light || !light->IsEnabled()) continue;

            Vector3 pos = light->GetPosition();
            Vector3 color = light->GetColor();

            // ワールド行列を計算（位置とスケール）
            DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(m_lightSphereSize, m_lightSphereSize, m_lightSphereSize);
            DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
            DirectX::XMMATRIX world = scale * translation;

            DirectX::XMFLOAT4X4 worldMatrix;
            DirectX::XMStoreFloat4x4(&worldMatrix, world);

            // 定数バッファを更新
            D3D11_MAPPED_SUBRESOURCE mapped{};
            if (SUCCEEDED(context->Map(m_unlitConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
                auto* cb = static_cast<UnlitConstantBuffer*>(mapped.pData);
                cb->world = worldMatrix;
                cb->view = viewMatrix;
                cb->projection = projMatrix;
                cb->color = { color.x, color.y, color.z, 1.0f };
                context->Unmap(m_unlitConstantBuffer.Get(), 0);
            }

            // 球体メッシュを描画
            m_sphereMesh->Bind(context);
            m_sphereMesh->Draw(context);
        }

        // Spot Lights（ライト色の球体）
        auto spotLights = lightSystem->GetSpotLights();
        for (auto* light : spotLights) {
            if (!light || !light->IsEnabled()) continue;

            Vector3 pos = light->GetPosition();
            Vector3 color = light->GetColor();

            // ワールド行列を計算（位置とスケール）
            DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(m_lightSphereSize, m_lightSphereSize, m_lightSphereSize);
            DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
            DirectX::XMMATRIX world = scale * translation;

            DirectX::XMFLOAT4X4 worldMatrix;
            DirectX::XMStoreFloat4x4(&worldMatrix, world);

            // 定数バッファを更新
            D3D11_MAPPED_SUBRESOURCE mapped{};
            if (SUCCEEDED(context->Map(m_unlitConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
                auto* cb = static_cast<UnlitConstantBuffer*>(mapped.pData);
                cb->world = worldMatrix;
                cb->view = viewMatrix;
                cb->projection = projMatrix;
                cb->color = { color.x, color.y, color.z, 1.0f };
                context->Unmap(m_unlitConstantBuffer.Get(), 0);
            }

            // 球体メッシュを描画
            m_sphereMesh->Bind(context);
            m_sphereMesh->Draw(context);
        }

        // Directional Lights（ライト色の球体、設定位置に表示）
        auto directionalLights = lightSystem->GetDirectionalLights();
        for (auto* light : directionalLights) {
            if (!light || !light->IsEnabled()) continue;

            Vector3 color = light->GetColor();
            const auto& displayPos = m_directionalLightIconPos;

            // ワールド行列を計算（位置とスケール - DirectionalLightは大きめに表示）
            float dirLightScale = m_lightSphereSize * 1.5f;
            DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(dirLightScale, dirLightScale, dirLightScale);
            DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(displayPos.x, displayPos.y, displayPos.z);
            DirectX::XMMATRIX world = scale * translation;

            DirectX::XMFLOAT4X4 worldMatrix;
            DirectX::XMStoreFloat4x4(&worldMatrix, world);

            // 定数バッファを更新
            D3D11_MAPPED_SUBRESOURCE mapped{};
            if (SUCCEEDED(context->Map(m_unlitConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
                auto* cb = static_cast<UnlitConstantBuffer*>(mapped.pData);
                cb->world = worldMatrix;
                cb->view = viewMatrix;
                cb->projection = projMatrix;
                cb->color = { color.x, color.y, color.z, 1.0f };
                context->Unmap(m_unlitConstantBuffer.Get(), 0);
            }

            // 球体メッシュを描画
            m_sphereMesh->Bind(context);
            m_sphereMesh->Draw(context);
        }
    }

    //============================================================
    // Debug Object Management (Prefab-based Light Markers)
    //============================================================

    void DebugVisualizationSystem::SpawnDebugLightMarkers(SceneContext& ctx, LightSystem* lightSystem) {
        if (!m_isInitialized) return;
        if (!lightSystem) {
            Logger::Warn("DebugVisualizationSystem::SpawnDebugLightMarkers: lightSystem is null");
            return;
        }
        if (!ctx.m_scene) {
            Logger::Warn("DebugVisualizationSystem::SpawnDebugLightMarkers: SceneContext.m_scene is null");
            return;
        }

        // 既存のマーカーを削除
        ClearDebugLightMarkers(ctx);

        // DebugLight Prefabを使用してライトマーカーを生成
        // Game::DebugLight prefabは独自の見た目（SampleCubeMaterial + 回転アニメーション）を持ち、
        // ライト位置にデバッグ可視化用のGameObjectを配置する

        // Point Lightsのマーカーを生成
        auto pointLights = lightSystem->GetPointLights();
        for (auto* light : pointLights) {
            if (!light || !light->IsEnabled()) continue;

            Vector3 pos = light->GetPosition();

            Game::DebugLight::SpawnDesc desc(pos, m_lightSphereSize);
            auto* marker = Game::DebugLight::Spawn(ctx, desc);
            if (marker) {
                m_debugLightMarkers.push_back(marker);
                m_lightMarkerInfos.push_back({ marker, light });
            }
        }

        // Spot Lightsのマーカーを生成
        auto spotLights = lightSystem->GetSpotLights();
        for (auto* light : spotLights) {
            if (!light || !light->IsEnabled()) continue;

            Vector3 pos = light->GetPosition();

            Game::DebugLight::SpawnDesc desc(pos, m_lightSphereSize);
            auto* marker = Game::DebugLight::Spawn(ctx, desc);
            if (marker) {
                m_debugLightMarkers.push_back(marker);
                m_lightMarkerInfos.push_back({ marker, light });
            }
        }

        // Directional Lightsのマーカーを生成（設定位置に表示）
        auto directionalLights = lightSystem->GetDirectionalLights();
        for (auto* light : directionalLights) {
            if (!light || !light->IsEnabled()) continue;

            // DirectionalLightは設定位置に表示する（位置はXMFLOAT3からVector3に変換）
            Vector3 pos(m_directionalLightIconPos.x, m_directionalLightIconPos.y, m_directionalLightIconPos.z);

            // DirectionalLightは大きめに表示
            float dirLightSize = m_lightSphereSize * 1.5f;

            Game::DebugLight::SpawnDesc desc(pos, dirLightSize);
            auto* marker = Game::DebugLight::Spawn(ctx, desc);
            if (marker) {
                m_debugLightMarkers.push_back(marker);
                m_lightMarkerInfos.push_back({ marker, light });
            }
        }

        Logger::Info("DebugVisualizationSystem: Spawned " + std::to_string(m_debugLightMarkers.size()) + " debug light markers.");
    }

    void DebugVisualizationSystem::ClearDebugLightMarkers(SceneContext& ctx) {
        if (m_debugLightMarkers.empty() && m_lightMarkerInfos.empty()) return;

        if (!ctx.m_scene) {
            Logger::Warn("DebugVisualizationSystem::ClearDebugLightMarkers: SceneContext.m_scene is null");
            m_debugLightMarkers.clear();
            m_lightMarkerInfos.clear();
            return;
        }

        // 全てのデバッグマーカーを削除
        for (auto* marker : m_debugLightMarkers) {
            if (marker) {
                ctx.m_scene->DestroyObject(marker);
            }
        }

        std::size_t count = m_debugLightMarkers.size();
        m_debugLightMarkers.clear();
        m_lightMarkerInfos.clear();

        Logger::Info("DebugVisualizationSystem: Cleared " + std::to_string(count) + " debug light markers.");
    }

    void DebugVisualizationSystem::UpdateDebugLightMarkerPositions(LightSystem* lightSystem) {
        if (!m_isInitialized) return;
        if (!lightSystem) {
            Logger::Warn("DebugVisualizationSystem::UpdateDebugLightMarkerPositions: lightSystem is null");
            return;
        }
        if (m_lightMarkerInfos.empty()) return;

        // 各マーカーの関連付けられたライトから位置を更新
        for (auto& info : m_lightMarkerInfos) {
            if (!info.m_marker || !info.m_light) continue;
            if (!info.m_light->IsEnabled()) {
                // ライトが無効になった場合はマーカーも非表示にする
                info.m_marker->SetActive(false);
                continue;
            }

            // マーカーを有効化（ライトが再度有効になった場合）
            if (!info.m_marker->IsActive()) {
                info.m_marker->SetActive(true);
            }

            // ライトタイプに応じて位置を取得
            LightType lightType = info.m_light->GetLightType();

            if (lightType == LightType::Point) {
                auto* pointLight = dynamic_cast<PointLightObject*>(info.m_light);
                if (pointLight) {
                    Vector3 pos = pointLight->GetPosition();
                    info.m_marker->SetPosition(pos);
                }
                else {
                    Logger::Warn("DebugVisualizationSystem::UpdateDebugLightMarkerPositions: PointLight dynamic_cast failed");
                }
            }
            else if (lightType == LightType::Spot) {
                auto* spotLight = dynamic_cast<SpotLightObject*>(info.m_light);
                if (spotLight) {
                    Vector3 pos = spotLight->GetPosition();
                    info.m_marker->SetPosition(pos);
                }
                else {
                    Logger::Warn("DebugVisualizationSystem::UpdateDebugLightMarkerPositions: SpotLight dynamic_cast failed");
                }
            }
            else if (lightType == LightType::Directional) {
                // DirectionalLightは設定位置に表示（XMFLOAT3からVector3への変換）
                Vector3 pos(m_directionalLightIconPos.x, m_directionalLightIconPos.y, m_directionalLightIconPos.z);
                info.m_marker->SetPosition(pos);
            }
        }
    }

    std::size_t DebugVisualizationSystem::GetDebugLightMarkerCount() const {
        return m_debugLightMarkers.size();
    }

    void DebugVisualizationSystem::SetUsePrefabLightMarkers(bool use) {
        m_usePrefabLightMarkers = use;
    }

    bool DebugVisualizationSystem::IsUsingPrefabLightMarkers() const {
        return m_usePrefabLightMarkers;
    }

} // namespace Engine
