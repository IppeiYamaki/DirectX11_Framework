#include "WaterSurfaceComponent.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Graphics/PlanarReflectionSystem.h"
#include "Engine/Graphics/Material.h"
#include "Engine/Graphics/Mesh.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Resources/AssetManager.h"

namespace Engine {

    WaterSurfaceComponent::WaterSurfaceComponent(ID3D11Device* device, RenderSystem* renderSystem, AssetManager* assetManager)
        : m_device(device)
        , m_renderSystem(renderSystem)
        , m_assetManager(assetManager) {
    }

    WaterSurfaceComponent::~WaterSurfaceComponent() {
        m_waterCb.Destroy();
        m_reflectionSampler.Reset();
    }

    void WaterSurfaceComponent::OnStart() {
        if (!m_device) {
            Logger::Error("WaterSurfaceComponent::OnStart: device is null.");
            return;
        }

        // 定数バッファを作成
        if (!m_waterCb.Create(m_device)) {
            Logger::Error("WaterSurfaceComponent::OnStart: failed to create water constant buffer.");
            return;
        }

        // 反射用サンプラーを作成
        if (!CreateReflectionSampler()) {
            Logger::Error("WaterSurfaceComponent::OnStart: failed to create reflection sampler.");
            return;
        }

        // Sky.pngを反射テクスチャとして読み込む
        if (m_assetManager) {
            m_reflectionTexture = m_assetManager->LoadTexture(L"Textures/Environment/Sky/sky.png");
            if (m_reflectionTexture && m_reflectionTexture->IsLoaded()) {
                Logger::Info("WaterSurfaceComponent: Sky.png loaded as reflection texture.");
            } else {
                Logger::Warn("WaterSurfaceComponent: Failed to load Sky.png. Water reflection may not work.");
            }
        }

        Logger::Info("WaterSurfaceComponent initialized.");
    }

    bool WaterSurfaceComponent::CreateReflectionSampler() {
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

        HRESULT hr = m_device->CreateSamplerState(&samplerDesc, m_reflectionSampler.GetAddressOf());
        return SUCCEEDED(hr);
    }

    void WaterSurfaceComponent::Update(float deltaTime) {
        m_elapsedTime += deltaTime;
        m_waterData.g_time = m_elapsedTime;
    }

    void WaterSurfaceComponent::Draw() {
        if (!m_renderSystem || !m_mesh || !m_material) {
            return;
        }

        auto* owner = GetOwner();
        if (!owner) return;

        auto* transform = owner->GetComponent<Transform>();
        if (!transform) return;

        // 水面用定数バッファを更新してバインド
        if (m_device) {
            ID3D11DeviceContext* ctx = nullptr;
            m_device->GetImmediateContext(&ctx);
            if (ctx) {
                m_waterCb.Update(ctx, m_waterData);
                ID3D11Buffer* b7 = m_waterCb.GetBuffer();
                ctx->VSSetConstantBuffers(7, 1, &b7);
                ctx->PSSetConstantBuffers(7, 1, &b7);

                // 反射テクスチャをバインド（優先順位: 静的テクスチャ > PlanarReflectionSystem RT）
                ID3D11ShaderResourceView* reflectionSRV = nullptr;
                
                // まず静的反射テクスチャ（Sky.png）を確認
                if (m_reflectionTexture && m_reflectionTexture->IsLoaded()) {
                    reflectionSRV = m_reflectionTexture->GetShaderResourceView();
                }
                // 静的テクスチャがなければPlanarReflectionSystemのRTを使用（将来の実装用）
                else if (m_reflectionSystem) {
                    reflectionSRV = m_reflectionSystem->GetReflectionSRV();
                }
                
                if (reflectionSRV) {
                    ctx->PSSetShaderResources(1, 1, &reflectionSRV);
                }
                if (m_reflectionSampler) {
                    ID3D11SamplerState* sampler = m_reflectionSampler.Get();
                    ctx->PSSetSamplers(1, 1, &sampler);
                }

                ctx->Release();
            }
        }

        // RenderItemを作成して追加
        RenderItem item;
        item.m_mesh = m_mesh.get();
        item.m_material = m_material.get();
        item.m_world = transform->GetWorldMatrix();
        item.m_layer = RenderLayer::Transparent;
        item.m_orderInLayer = 100;  // 水面は透明オブジェクトの後に描画
        item.m_stateFlags = kRenderStateBlendAlpha | kRenderStateCullNone;
        item.m_excludeFromReflection = true;  // 水面自身は反射パスから除外

        m_renderSystem->AddRenderItem(item);
    }

    float WaterSurfaceComponent::GetWaterHeight() const {
        auto* owner = GetOwner();
        if (!owner) return 0.0f;

        auto* transform = owner->GetComponent<Transform>();
        if (!transform) return 0.0f;

        return transform->GetPosition().y;
    }

} // namespace Engine
