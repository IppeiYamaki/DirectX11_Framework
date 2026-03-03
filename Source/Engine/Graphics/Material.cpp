#include "Material.h"

#include <typeinfo>

#include "Engine/Core/Logger.h"
#include "Engine/Core/Assert.h"

namespace Engine {

    namespace {
        // Logging throttle: number of frames between debug log outputs
        constexpr int kLogThrottleFrames = 300;
    }

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

        // Default params
        m_materialData.g_material = MaterialParams{};
        m_materialData.g_material.m_baseColor = Vector4(1, 1, 1, 1);
        m_materialData.g_material.m_ambient = Vector4(1, 1, 1, 1);
        m_materialData.g_material.m_specular = Vector4(0, 0, 0, 0);
        m_materialData.g_material.m_emissive = Vector4(0, 0, 0, 0);
        m_materialData.g_material.m_flags = 0;
        m_materialData.g_material.m_alphaCutoff = 0.5f;

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

        // TerrainBlend cleanup
        m_terrainBlendCb.reset();
        m_useTerrainBlend = false;

        // Terrain layer textures cleanup
        for (auto& tex : m_terrainLayerTextures) {
            tex.reset();
        }

        m_isInitialized = false;
        m_firstBindLogged = false;
    }

    bool Material::IsInitialized() const {
        return m_isInitialized;
    }

    void Material::SetVertexShader(const std::shared_ptr<VertexShader>& vs) { m_vs = vs; }
    void Material::SetPixelShader(const std::shared_ptr<PixelShader>& ps) { m_ps = ps; }
    void Material::SetInputLayout(const std::shared_ptr<InputLayout>& il) { m_inputLayout = il; }

    void Material::SetTexture(const std::shared_ptr<Texture>& texture) {
        m_texture = texture;

        if (m_texture && m_texture->IsLoaded() && m_texture->GetShaderResourceView()) {
            m_materialData.g_material.m_flags |= kMaterialFlagUseTexture;
        }
        else {
            m_materialData.g_material.m_flags &= ~kMaterialFlagUseTexture;
        }
    }

    void Material::SetSampler(ID3D11SamplerState* sampler) {
        m_samplerExternal = sampler;
    }

    MaterialParams& Material::GetParams() { return m_materialData.g_material; }
    const MaterialParams& Material::GetParams() const { return m_materialData.g_material; }

    void Material::EnableTexture(bool isEnabled) {
        if (isEnabled) m_materialData.g_material.m_flags |= kMaterialFlagUseTexture;
        else           m_materialData.g_material.m_flags &= ~kMaterialFlagUseTexture;
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

    bool Material::EnableTerrainBlendBuffer(ID3D11Device* device) {
        Logger::Info("Material::EnableTerrainBlendBuffer - Starting (device=" + 
            std::to_string(reinterpret_cast<uintptr_t>(device)) + ")");
        
        if (!device) {
            Logger::Error("Material::EnableTerrainBlendBuffer failed: device is null.");
            return false;
        }

        if (!m_terrainBlendCb) {
            Logger::Info("Material::EnableTerrainBlendBuffer - Creating constant buffer");
            m_terrainBlendCb = std::make_unique<ConstantBuffer<TerrainBlendCB>>();
            if (!m_terrainBlendCb->Create(device)) {
                Logger::Error("Material::EnableTerrainBlendBuffer failed: create constant buffer failed.");
                m_terrainBlendCb.reset();
                m_useTerrainBlend = false;
                return false;
            }
            Logger::Info("Material::EnableTerrainBlendBuffer - Constant buffer created successfully");
        }

        // デフォルト値を設定（レイヤー0のみ有効、白色テクスチャブレンド）
        m_terrainBlendData.g_terrainBlend = TerrainBlendParams{};
        m_terrainBlendData.g_terrainBlend.m_activeLayerCount = 1;
        m_terrainBlendData.g_terrainBlend.m_blendWeights = Vector4(1.0f, 0.0f, 0.0f, 0.0f);

        // CRITICAL: Set the terrain blend flag to true
        m_useTerrainBlend = true;
        
        Logger::Info("Material::EnableTerrainBlendBuffer - SUCCESS: m_useTerrainBlend=" + 
            std::to_string(m_useTerrainBlend ? 1 : 0) + 
            ", cbValid=" + std::to_string(m_terrainBlendCb && m_terrainBlendCb->IsValid() ? 1 : 0));
        return true;
    }

    TerrainBlendParams& Material::GetTerrainBlendParams() {
        return m_terrainBlendData.g_terrainBlend;
    }

    const TerrainBlendParams& Material::GetTerrainBlendParams() const {
        return m_terrainBlendData.g_terrainBlend;
    }

    void Material::SetTerrainLayerTexture(std::uint32_t layerIndex, const std::shared_ptr<Texture>& texture) {
        if (layerIndex >= kMaxTerrainLayers) {
            Logger::Warn("Material::SetTerrainLayerTexture - layerIndex out of range: " + std::to_string(layerIndex));
            return;
        }
        m_terrainLayerTextures[layerIndex] = texture;
        
        // Log texture assignment for debugging
        if (texture && texture->IsLoaded()) {
            std::string narrowPath(texture->GetFilePath().begin(), texture->GetFilePath().end());
            Logger::Info("Material::SetTerrainLayerTexture[" + std::to_string(layerIndex) + "] = " + narrowPath +
                ", SRV=" + std::to_string(reinterpret_cast<uintptr_t>(texture->GetShaderResourceView())));
        } else {
            Logger::Warn("Material::SetTerrainLayerTexture[" + std::to_string(layerIndex) + "] - texture is null or not loaded");
        }
        
        // When setting terrain layer textures, ensure terrain blend mode is enabled
        // This provides a safeguard in case EnableTerrainBlendBuffer was not called
        if (!m_useTerrainBlend) {
            Logger::Warn("Material::SetTerrainLayerTexture - m_useTerrainBlend was false! Terrain layer textures require TerrainBlend mode. "
                "Check that EnableTerrainBlendBuffer was called and succeeded.");
        }
    }

    std::shared_ptr<Texture> Material::GetTerrainLayerTexture(std::uint32_t layerIndex) const {
        if (layerIndex >= kMaxTerrainLayers) {
            return nullptr;
        }
        return m_terrainLayerTextures[layerIndex];
    }

    void Material::Bind(ID3D11DeviceContext* ctx) {
        if (!m_isInitialized || ctx == nullptr) return;

        // Per-instance first bind log to help diagnose TerrainBlend issues
        // typeid(*this).name() output is implementation-defined and may produce mangled names,
        // but is useful for debugging to verify polymorphic dispatch is working correctly
        if (!m_firstBindLogged) {
            m_firstBindLogged = true;
            Logger::Info("[INFO] Material::Bind called: type=" + std::string(typeid(*this).name()) + 
                ", this=" + std::to_string(reinterpret_cast<uintptr_t>(this)) +
                ", useTerrainBlend=" + std::to_string(m_useTerrainBlend ? 1 : 0) +
                ", activeLayerCount=" + std::to_string(m_terrainBlendData.g_terrainBlend.m_activeLayerCount) +
                ", hasCb=" + std::to_string(m_terrainBlendCb ? 1 : 0) +
                ", cbValid=" + std::to_string((m_terrainBlendCb && m_terrainBlendCb->IsValid()) ? 1 : 0));
        }

        // Pipeline
        if (m_inputLayout) ctx->IASetInputLayout(m_inputLayout->GetInputLayout());
        if (m_vs) ctx->VSSetShader(m_vs->GetShader(), nullptr, 0);
        if (m_ps) ctx->PSSetShader(m_ps->GetShader(), nullptr, 0);

        // b3
        m_materialCb.Update(ctx, m_materialData);
        ID3D11Buffer* b3 = m_materialCb.GetBuffer();
        ctx->VSSetConstantBuffers(3, 1, &b3);
        ctx->PSSetConstantBuffers(3, 1, &b3);

        // b7 (TerrainBlend - optional)
        if (m_useTerrainBlend && m_terrainBlendCb && m_terrainBlendCb->IsValid()) {
            m_terrainBlendCb->Update(ctx, m_terrainBlendData);
            ID3D11Buffer* b7 = m_terrainBlendCb->GetBuffer();
            ctx->PSSetConstantBuffers(7, 1, &b7);

            // Bind terrain layer textures (t0-t3)
            ID3D11ShaderResourceView* srvs[kMaxTerrainLayers] = { nullptr };
            std::uint32_t boundCount = 0;
            for (std::uint32_t i = 0; i < kMaxTerrainLayers; ++i) {
                if (m_terrainLayerTextures[i] && m_terrainLayerTextures[i]->IsLoaded()) {
                    srvs[i] = m_terrainLayerTextures[i]->GetShaderResourceView();
                    if (srvs[i]) ++boundCount;
                }
            }
            ctx->PSSetShaderResources(0, kMaxTerrainLayers, srvs);
            
            // Debug log (throttled to avoid spam)
            static int s_bindLogCounter = 0;
            if (++s_bindLogCounter >= kLogThrottleFrames) {
                s_bindLogCounter = 0;
                Logger::Info("Material::Bind TerrainBlend: activeLayerCount=" + 
                    std::to_string(m_terrainBlendData.g_terrainBlend.m_activeLayerCount) +
                    ", boundTextures=" + std::to_string(boundCount) +
                    ", weights=(" + std::to_string(m_terrainBlendData.g_terrainBlend.m_blendWeights.x) + 
                    "," + std::to_string(m_terrainBlendData.g_terrainBlend.m_blendWeights.y) + 
                    "," + std::to_string(m_terrainBlendData.g_terrainBlend.m_blendWeights.z) + 
                    "," + std::to_string(m_terrainBlendData.g_terrainBlend.m_blendWeights.w) + ")");
            }
        }
        else {
            // Non-terrain mode: bind single texture to t0
            // Log why terrain blend was not used (throttled)
            static int s_nonTerrainLogCounter = 0;
            if (++s_nonTerrainLogCounter >= kLogThrottleFrames) {
                s_nonTerrainLogCounter = 0;
                Logger::Info("Material::Bind Non-TerrainBlend mode: m_useTerrainBlend=" + 
                    std::to_string(m_useTerrainBlend ? 1 : 0) +
                    ", hasCb=" + std::to_string(m_terrainBlendCb ? 1 : 0) +
                    ", cbValid=" + std::to_string((m_terrainBlendCb && m_terrainBlendCb->IsValid()) ? 1 : 0));
            }
            
            ID3D11ShaderResourceView* srv = nullptr;
            if (m_texture && m_texture->IsLoaded()) {
                srv = m_texture->GetShaderResourceView();
            }
            ctx->PSSetShaderResources(0, 1, &srv);
        }

        // s0
        ID3D11SamplerState* sampler = m_samplerExternal ? m_samplerExternal : m_defaultSampler.Get();
        ctx->PSSetSamplers(0, 1, &sampler);
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
