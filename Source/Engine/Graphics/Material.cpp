#include "Material.h"

#include "Engine/Core/Logger.h"
#include "Engine/Core/Assert.h"

namespace Engine {

    Material::~Material() {
        Finalize();
    }

    bool Material::Initialize(ID3D11Device* device) {
        if (m_isInitialized) return true;

        if (device == nullptr) {
            Logger::Error("Material::Initialize failed: device is null.");
            return false;
        }

        if (!m_materialCb.Create(device)) {
            Logger::Error("Material::Initialize failed: create constant buffer failed.");
            Finalize();
            return false;
        }

        if (!CreateDefaultSampler(device)) {
            Logger::Error("Material::Initialize failed: create default sampler failed.");
            Finalize();
            return false;
        }

        // デフォルト値
        m_materialData.g_material = MaterialParams{};
        m_materialData.g_material.m_baseColor = Vector4(1, 1, 1, 1);
        m_materialData.g_material.m_ambient = Vector4(1, 1, 1, 1);
        m_materialData.g_material.m_emissive = Vector4(0, 0, 0, 0);
        m_materialData.g_material.m_flags = 0;

        m_isInitialized = true;
        return true;
    }

    void Material::Finalize() {
        m_vs.reset();
        m_ps.reset();
        m_inputLayout.reset();
        m_texture.reset();

        m_samplerExternal = nullptr;
        m_defaultSampler.Reset();

        m_materialCb.Destroy();

        m_isInitialized = false;
    }

    bool Material::IsInitialized() const {
        return m_isInitialized;
    }

    void Material::SetVertexShader(const std::shared_ptr<VertexShader>& vs) {
        m_vs = vs;
    }

    void Material::SetPixelShader(const std::shared_ptr<PixelShader>& ps) {
        m_ps = ps;
    }

    void Material::SetInputLayout(const std::shared_ptr<InputLayout>& inputLayout) {
        m_inputLayout = inputLayout;
    }

    void Material::SetTexture(const std::shared_ptr<Texture>& texture) {
        m_texture = texture;

        // テクスチャが入ったらフラグを立てる（無ければ降ろす）
        if (m_texture && m_texture->IsLoaded()) {
            m_materialData.g_material.m_flags |= kMaterialFlagUseTexture;
        }
        else {
            m_materialData.g_material.m_flags &= ~kMaterialFlagUseTexture;
        }
    }

    void Material::SetSampler(ID3D11SamplerState* sampler) {
        m_samplerExternal = sampler;
    }

    MaterialParams& Material::GetParams() {
        return m_materialData.g_material;
    }

    const MaterialParams& Material::GetParams() const {
        return m_materialData.g_material;
    }

    void Material::EnableTexture(bool isEnabled) {
        if (isEnabled) {
            m_materialData.g_material.m_flags |= kMaterialFlagUseTexture;
        }
        else {
            m_materialData.g_material.m_flags &= ~kMaterialFlagUseTexture;
        }
    }

    void Material::EnableAlphaTest(bool isEnabled, float alphaCutoff) {
        if (isEnabled) {
            m_materialData.g_material.m_flags |= kMaterialFlagAlphaTest;
            m_materialData.g_material.m_alphaCutoff = alphaCutoff;
        }
        else {
            m_materialData.g_material.m_flags &= ~kMaterialFlagAlphaTest;
        }
    }

    void Material::Bind(ID3D11DeviceContext* ctx) {
        if (!m_isInitialized) return;
        if (ctx == nullptr) return;

        // シェーダ＆レイアウト
        if (m_inputLayout) {
            ctx->IASetInputLayout(m_inputLayout->GetInputLayout());
        }
        if (m_vs) {
            ctx->VSSetShader(m_vs->GetShader(), nullptr, 0);
        }
        if (m_ps) {
            ctx->PSSetShader(m_ps->GetShader(), nullptr, 0);
        }

        // テクスチャフラグを実体に合わせて整合（事故防止）
        const bool hasTexture = (m_texture && m_texture->IsLoaded() && m_texture->GetShaderResourceView() != nullptr);
        if (hasTexture) {
            m_materialData.g_material.m_flags |= kMaterialFlagUseTexture;
        }
        else {
            m_materialData.g_material.m_flags &= ~kMaterialFlagUseTexture;
        }

        // b3 更新＆バインド（VS/PS両方に）
        m_materialCb.Update(ctx, m_materialData);
        ID3D11Buffer* b3 = m_materialCb.GetBuffer();
        ctx->VSSetConstantBuffers(3, 1, &b3);
        ctx->PSSetConstantBuffers(3, 1, &b3);

        // t0 / s0
        ID3D11ShaderResourceView* srv = hasTexture ? m_texture->GetShaderResourceView() : nullptr;
        ctx->PSSetShaderResources(0, 1, &srv);

        ID3D11SamplerState* sampler = m_samplerExternal ? m_samplerExternal : m_defaultSampler.Get();
        ctx->PSSetSamplers(0, 1, &sampler);
    }

    ID3D11InputLayout* Material::GetInputLayout() const {
        return m_inputLayout ? m_inputLayout->GetInputLayout() : nullptr;
    }

    bool Material::CreateDefaultSampler(ID3D11Device* device) {
        D3D11_SAMPLER_DESC desc{};
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        desc.MinLOD = 0;
        desc.MaxLOD = D3D11_FLOAT32_MAX;

        const HRESULT hr = device->CreateSamplerState(&desc, m_defaultSampler.GetAddressOf());
        if (FAILED(hr)) {
            Engine::Assert::ReportHrFailure(hr, "CreateSamplerState(Material default)", __FILE__, __LINE__, __func__);
            m_defaultSampler.Reset();
            return false;
        }
        return true;
    }

} // namespace Engine
