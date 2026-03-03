/// @file   TerrainBlendMaterial.cpp
/// @brief  地形用マルチテクスチャブレンドマテリアルの実装
#define NOMINMAX
#include "TerrainBlendMaterial.h"

#include <algorithm>
#include <cstddef>
#include <typeinfo>

#include "Engine/Core/Logger.h"
#include "Engine/Resources/AssetManager.h"
#include "Engine/Graphics/Shader.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"

namespace Engine {

    namespace {
        // Logging throttle: number of frames between periodic Bind debug log outputs
        constexpr int kBindLogThrottleFrames = 300;
    }

    //=========================================================================
    // 頂点レイアウト（DefaultVSと同じPos/Normal/Color/UV）
    //=========================================================================
    struct VertexPosNormColorUv_Terrain {
        Vector3 m_pos;
        Vector3 m_normal;
        Vector4 m_color;
        Vector2 m_uv;
    };

    static VertexInputLayout CreateTerrainInputLayout() {
        VertexInputLayout layout;
        layout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, (UINT)offsetof(VertexPosNormColorUv_Terrain, m_pos),    D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layout.push_back({ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, (UINT)offsetof(VertexPosNormColorUv_Terrain, m_normal), D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layout.push_back({ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, (UINT)offsetof(VertexPosNormColorUv_Terrain, m_color),  D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, (UINT)offsetof(VertexPosNormColorUv_Terrain, m_uv),     D3D11_INPUT_PER_VERTEX_DATA, 0 });
        return layout;
    }

    //=========================================================================
    // TerrainBlendMaterial Implementation
    //=========================================================================

    TerrainBlendMaterial::~TerrainBlendMaterial() {
        Finalize();
    }

    bool TerrainBlendMaterial::Initialize(ID3D11Device* device) {
        if (!device) {
            Logger::Error("TerrainBlendMaterial::Initialize - device is null");
            return false;
        }

        // 基底クラス Material の初期化を呼び出す
        if (!Material::Initialize(device)) {
            Logger::Error("TerrainBlendMaterial::Initialize - Material::Initialize failed");
            return false;
        }

        // TerrainBlend定数バッファを有効化（基底クラスのb7バッファを使用）
        if (!EnableTerrainBlendBuffer(device)) {
            Logger::Error("TerrainBlendMaterial::Initialize - EnableTerrainBlendBuffer failed");
            return false;
        }

        // パラメータを初期化
        m_terrainParams = TerrainBlendParams{};
        m_terrainParams.m_activeLayerCount = 1;
        m_terrainParams.m_blendWeights = Vector4(1.0f, 0.0f, 0.0f, 0.0f);

        m_isDirty = true;
        m_terrainFirstBindLogged = false;

        Logger::Info("TerrainBlendMaterial::Initialize - SUCCESS, this=" + 
            std::to_string(reinterpret_cast<uintptr_t>(this)) + 
            ", type=" + std::string(typeid(*this).name()));
        return true;
    }

    void TerrainBlendMaterial::Finalize() {
        // 基底クラスのFinalizeを呼び出す
        Material::Finalize();
        m_terrainFirstBindLogged = false;
    }

    //=========================================================================
    // Texture Layer Management
    //=========================================================================

    void TerrainBlendMaterial::SetLayerTexture(std::uint32_t layerIndex, const std::shared_ptr<Texture>& texture) {
        // 基底クラスのSetTerrainLayerTextureを使用
        SetTerrainLayerTexture(layerIndex, texture);
        
        if (texture && texture->IsLoaded()) {
            std::string narrowPath(texture->GetFilePath().begin(), texture->GetFilePath().end());
            Logger::Info("TerrainBlendMaterial::SetLayerTexture[" + std::to_string(layerIndex) + "] = " + narrowPath);
        }
    }

    std::shared_ptr<Texture> TerrainBlendMaterial::GetLayerTexture(std::uint32_t layerIndex) const {
        // 基底クラスのGetTerrainLayerTextureを使用
        return GetTerrainLayerTexture(layerIndex);
    }

    //=========================================================================
    // Blend Weight Management
    //=========================================================================

    void TerrainBlendMaterial::SetBlendWeights(float w0, float w1, float w2, float w3) {
        // 入力値をログ出力
        Logger::Info("[INFO] TerrainBlendMaterial::SetBlendWeights called with input: (" + 
            std::to_string(w0) + ", " + std::to_string(w1) + ", " + 
            std::to_string(w2) + ", " + std::to_string(w3) + ")");
        
        // 正規化（合計が1.0になるように）
        float sum = w0 + w1 + w2 + w3;
        if (sum > 0.0001f) {
            w0 /= sum;
            w1 /= sum;
            w2 /= sum;
            w3 /= sum;
        } else {
            // デフォルト：レイヤー0のみ
            w0 = 1.0f;
            w1 = w2 = w3 = 0.0f;
        }
        m_terrainParams.m_blendWeights = Vector4(w0, w1, w2, w3);
        
        // 基底クラスのTerrainBlendParamsも同期
        GetTerrainBlendParams().m_blendWeights = m_terrainParams.m_blendWeights;
        
        // 正規化後の値をログ出力（m_terrainParamsから取得して正確な値を表示）
        Logger::Info("[INFO] TerrainBlendMaterial::SetBlendWeights normalized to: (" + 
            std::to_string(m_terrainParams.m_blendWeights.x) + ", " + 
            std::to_string(m_terrainParams.m_blendWeights.y) + ", " + 
            std::to_string(m_terrainParams.m_blendWeights.z) + ", " + 
            std::to_string(m_terrainParams.m_blendWeights.w) + ")");
        
        m_isDirty = true;
    }

    void TerrainBlendMaterial::SetLayerBlendWeight(std::uint32_t layerIndex, float weight) {
        if (layerIndex >= kMaxTerrainLayers) {
            Logger::Warn("TerrainBlendMaterial::SetLayerBlendWeight - layerIndex out of range: " + std::to_string(layerIndex));
            return;
        }
        
        // 個別設定（正規化は行わない）
        switch (layerIndex) {
            case 0: m_terrainParams.m_blendWeights.x = weight; break;
            case 1: m_terrainParams.m_blendWeights.y = weight; break;
            case 2: m_terrainParams.m_blendWeights.z = weight; break;
            case 3: m_terrainParams.m_blendWeights.w = weight; break;
        }
        
        // 基底クラスのTerrainBlendParamsも同期
        GetTerrainBlendParams().m_blendWeights = m_terrainParams.m_blendWeights;
        
        m_isDirty = true;
    }

    const Vector4& TerrainBlendMaterial::GetBlendWeights() const {
        return m_terrainParams.m_blendWeights;
    }

    //=========================================================================
    // UV Scale Management
    //=========================================================================

    void TerrainBlendMaterial::SetUVScales(float scale0, float scale1, float scale2, float scale3) {
        m_terrainParams.m_uvScales = Vector4(scale0, scale1, scale2, scale3);
        
        // 基底クラスのTerrainBlendParamsも同期
        GetTerrainBlendParams().m_uvScales = m_terrainParams.m_uvScales;
        
        m_isDirty = true;
    }

    void TerrainBlendMaterial::SetLayerUVScale(std::uint32_t layerIndex, float scale) {
        if (layerIndex >= kMaxTerrainLayers) {
            Logger::Warn("TerrainBlendMaterial::SetLayerUVScale - layerIndex out of range: " + std::to_string(layerIndex));
            return;
        }
        
        switch (layerIndex) {
            case 0: m_terrainParams.m_uvScales.x = scale; break;
            case 1: m_terrainParams.m_uvScales.y = scale; break;
            case 2: m_terrainParams.m_uvScales.z = scale; break;
            case 3: m_terrainParams.m_uvScales.w = scale; break;
        }
        
        // 基底クラスのTerrainBlendParamsも同期
        GetTerrainBlendParams().m_uvScales = m_terrainParams.m_uvScales;
        
        m_isDirty = true;
    }

    const Vector4& TerrainBlendMaterial::GetUVScales() const {
        return m_terrainParams.m_uvScales;
    }

    //=========================================================================
    // Layer Tint Management
    //=========================================================================

    void TerrainBlendMaterial::SetLayerTint(std::uint32_t layerIndex, const Vector4& tint) {
        if (layerIndex >= kMaxTerrainLayers) {
            Logger::Warn("TerrainBlendMaterial::SetLayerTint - layerIndex out of range: " + std::to_string(layerIndex));
            return;
        }
        m_terrainParams.m_layerTints[layerIndex] = tint;
        
        // 基底クラスのTerrainBlendParamsも同期
        GetTerrainBlendParams().m_layerTints[layerIndex] = tint;
        
        m_isDirty = true;
    }

    const Vector4& TerrainBlendMaterial::GetLayerTint(std::uint32_t layerIndex) const {
        static const Vector4 kDefaultTint(1.0f, 1.0f, 1.0f, 1.0f);
        if (layerIndex >= kMaxTerrainLayers) {
            return kDefaultTint;
        }
        return m_terrainParams.m_layerTints[layerIndex];
    }

    //=========================================================================
    // Active Layer Count
    //=========================================================================

    void TerrainBlendMaterial::SetActiveLayerCount(std::uint32_t count) {
        m_terrainParams.m_activeLayerCount = std::min(count, kMaxTerrainLayers);
        
        // 基底クラスのTerrainBlendParamsも同期
        GetTerrainBlendParams().m_activeLayerCount = m_terrainParams.m_activeLayerCount;
        
        m_isDirty = true;
    }

    std::uint32_t TerrainBlendMaterial::GetActiveLayerCount() const {
        return m_terrainParams.m_activeLayerCount;
    }

    //=========================================================================
    // Binding (override from Material)
    //=========================================================================

    void TerrainBlendMaterial::UpdateConstantBuffer(ID3D11DeviceContext* context) {
        if (!m_isDirty) return;
        
        // 基底クラスのTerrainBlendParamsを更新
        auto& baseParams = GetTerrainBlendParams();
        baseParams = m_terrainParams;
        
        m_isDirty = false;
    }

    void TerrainBlendMaterial::Bind(ID3D11DeviceContext* context) {
        if (!m_isInitialized || context == nullptr) return;

        // ローカルパラメータを基底クラスに同期してからBindを呼ぶ
        UpdateConstantBuffer(context);

        // Per-instance first bind log - include typeid to verify polymorphic dispatch
        // Also log actual blend weights being sent to constant buffer
        if (!m_terrainFirstBindLogged) {
            m_terrainFirstBindLogged = true;
            Logger::Info("[INFO] TerrainBlendMaterial::Bind (first): type=" + std::string(typeid(*this).name()) + 
                ", this=" + std::to_string(reinterpret_cast<uintptr_t>(this)) +
                ", useTerrainBlend=" + std::to_string(IsTerrainBlendEnabled() ? 1 : 0) +
                ", activeLayerCount=" + std::to_string(m_terrainParams.m_activeLayerCount));
            
            // バインド直前のblendWeights値をログ出力（デバッグ用）
            Logger::Info("[INFO] Material::Bind: blendWeights = (" + 
                std::to_string(m_terrainParams.m_blendWeights.x) + ", " + 
                std::to_string(m_terrainParams.m_blendWeights.y) + ", " + 
                std::to_string(m_terrainParams.m_blendWeights.z) + ", " + 
                std::to_string(m_terrainParams.m_blendWeights.w) + ")");
        }

        // 基底クラスのBind処理を呼び出す（シェーダー、テクスチャ、定数バッファのバインド）
        Material::Bind(context);
        
        // Throttled debug logging (Note: static counter is not thread-safe, but rendering is typically single-threaded)
        static int s_bindLogCounter = 0;
        if (++s_bindLogCounter >= kBindLogThrottleFrames) {
            s_bindLogCounter = 0;
            Logger::Info("[INFO] Material::Bind: blendWeights = (" + 
                std::to_string(m_terrainParams.m_blendWeights.x) + ", " + 
                std::to_string(m_terrainParams.m_blendWeights.y) + ", " + 
                std::to_string(m_terrainParams.m_blendWeights.z) + ", " + 
                std::to_string(m_terrainParams.m_blendWeights.w) + ") [periodic]");
        }
    }

    //=========================================================================
    // Factory Function for MaterialLibrary
    //=========================================================================

    std::shared_ptr<Material> TerrainBlendMaterial::Create(const MaterialBuildContext& ctx) {
        if (!ctx.m_device || !ctx.m_assets) {
            Logger::Error("TerrainBlendMaterial::Create failed: ctx invalid.");
            return nullptr;
        }

        Logger::Info("TerrainBlendMaterial::Create - Starting TerrainBlendMaterial creation (device=" + 
            std::to_string(reinterpret_cast<uintptr_t>(ctx.m_device)) + ")");

        // TerrainBlend用シェーダーをロード（なければDefaultを使用）
        auto vs = ctx.m_assets->LoadVertexShader(L"Shaders/TerrainBlendVS.cso");
        if (!vs) {
            // フォールバック：デフォルトVS
            Logger::Warn("TerrainBlendMaterial::Create - TerrainBlendVS.cso not found, using DefaultVS.cso");
            vs = ctx.m_assets->LoadVertexShader(L"Shaders/DefaultVS.cso");
        } else {
            Logger::Info("TerrainBlendMaterial::Create - Loaded TerrainBlendVS.cso successfully");
        }

        auto ps = ctx.m_assets->LoadPixelShader(L"Shaders/TerrainBlendPS.cso");
        if (!ps) {
            // フォールバック：デフォルトPS
            Logger::Warn("TerrainBlendMaterial::Create - TerrainBlendPS.cso not found, using DefaultPS.cso");
            ps = ctx.m_assets->LoadPixelShader(L"Shaders/DefaultPS.cso");
        } else {
            Logger::Info("TerrainBlendMaterial::Create - Loaded TerrainBlendPS.cso successfully");
        }

        if (!vs || !ps) {
            Logger::Error("TerrainBlendMaterial::Create - Failed to load shaders");
            return nullptr;
        }

        // InputLayout作成
        auto il = ctx.m_assets->CreateInputLayout(L"TerrainBlendPosNormColorUv", CreateTerrainInputLayout(), *vs);
        if (!il) {
            Logger::Error("TerrainBlendMaterial::Create - Failed to create input layout");
            return nullptr;
        }

        // TerrainBlendMaterialインスタンスを作成（継承によるポリモーフィズム）
        auto mat = std::make_shared<TerrainBlendMaterial>();
        if (!mat->Initialize(ctx.m_device)) {
            Logger::Error("TerrainBlendMaterial::Create - Failed to initialize TerrainBlendMaterial");
            return nullptr;
        }

        mat->SetVertexShader(vs);
        mat->SetPixelShader(ps);
        mat->SetInputLayout(il);

        // Material params
        mat->GetParams().m_baseColor = Vector4(1, 1, 1, 1);
        mat->EnableTexture(true);

        // Verify that TerrainBlend mode is enabled
        if (!mat->IsTerrainBlendEnabled()) {
            Logger::Error("TerrainBlendMaterial::Create - BUG: TerrainBlendMaterial created but IsTerrainBlendEnabled is false!");
            return nullptr;
        }

        Logger::Info("TerrainBlendMaterial::Create - SUCCESS: TerrainBlendMaterial created with polymorphic Bind, type=" + 
            std::string(typeid(*mat).name()) +
            ", IsTerrainBlendEnabled=" + std::to_string(mat->IsTerrainBlendEnabled() ? 1 : 0));
        return mat;
    }

    //=========================================================================
    // TerrainBlendMaterialAsset Implementation
    //=========================================================================

    std::shared_ptr<Material> TerrainBlendMaterialAsset::Create(const MaterialBuildContext& ctx) {
        Logger::Info("TerrainBlendMaterialAsset::Create - Delegating to TerrainBlendMaterial::Create");
        auto mat = TerrainBlendMaterial::Create(ctx);
        if (mat) {
            Logger::Info("TerrainBlendMaterialAsset::Create - Material created successfully, type=" + 
                std::string(typeid(*mat).name()) +
                ", IsTerrainBlendEnabled=" + std::to_string(mat->IsTerrainBlendEnabled() ? 1 : 0));
        } else {
            Logger::Error("TerrainBlendMaterialAsset::Create - Failed to create material");
        }
        return mat;
    }

} // namespace Engine
