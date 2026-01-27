# DirectX11 パフォーマンス最適化提案

## 概要

GraphicsDevice、RenderSystem、Shader関連機能の効率性向上とメモリ使用量削減のための具体的な最適化提案を示します。

---

## 現状分析

### 良い点 ✅

1. **ComPtr統一**: 手動Release()がなく、メモリリーク防止
2. **RenderItem分離**: 描画要求がデータとして管理されている
3. **レイヤー/オーダー対応**: RenderLayerによる描画順制御
4. **定数バッファ設計**: World/View/Projection/Lightが分離

### 改善可能な点 🔧

1. **描画ソート最適化**: シェーダー/マテリアル切替の最小化
2. **インスタンシング未対応**: 同一メッシュの重複描画
3. **定数バッファ更新**: 毎フレームの不要な更新
4. **リソースバインディング**: 冗長なステート設定

---

## 最適化提案

### 1. RenderItem ソート最適化（高優先度）

**目的:** GPU状態変更を最小化し、描画コール効率を向上

**現状の問題:**
```cpp
// RenderSystemで描画順のみソート
// シェーダー/マテリアル切替が頻発する可能性
```

**解決案:**
```cpp
// Engine/Graphics/RenderSystem.cpp

struct RenderSortKey {
    std::uint64_t key;

    static RenderSortKey Create(const RenderItem& item) {
        RenderSortKey sortKey;
        // ビット配分: [Layer:4][Shader:16][Material:16][Order:16][Depth:12]
        sortKey.key = 0;
        sortKey.key |= (static_cast<std::uint64_t>(item.m_layer) & 0xF) << 60;
        sortKey.key |= (GetShaderId(item.m_material) & 0xFFFF) << 44;
        sortKey.key |= (GetMaterialId(item.m_material) & 0xFFFF) << 28;
        sortKey.key |= (static_cast<std::uint64_t>(item.m_orderInLayer) & 0xFFFF) << 12;
        sortKey.key |= (CalculateDepth(item) & 0xFFF);
        return sortKey;
    }

    bool operator<(const RenderSortKey& other) const {
        return key < other.key;
    }
};

void RenderSystem::SortRenderItems() {
    // ソートキー生成
    std::vector<std::pair<RenderSortKey, size_t>> sortedIndices;
    sortedIndices.reserve(m_items.size());

    for (size_t i = 0; i < m_items.size(); ++i) {
        sortedIndices.emplace_back(RenderSortKey::Create(m_items[i]), i);
    }

    // ソート（シェーダー→マテリアル→オーダー順）
    std::sort(sortedIndices.begin(), sortedIndices.end(),
        [](const auto& a, const auto& b) { return a.first < b.first; });

    // 並び替え適用
    std::vector<RenderItem> sorted;
    sorted.reserve(m_items.size());
    for (const auto& [key, idx] : sortedIndices) {
        sorted.push_back(std::move(m_items[idx]));
    }
    m_items = std::move(sorted);
}
```

---

### 2. インスタンシング対応（高優先度）

**目的:** 同一メッシュ+マテリアルの描画を1回のDrawCallで実行

```cpp
// Engine/Graphics/InstanceData.h
#pragma once

#include <DirectXMath.h>
#include <vector>

namespace Engine {

    /// @brief インスタンスごとのデータ
    struct InstanceData {
        DirectX::XMFLOAT4X4 m_world;
        DirectX::XMFLOAT4   m_color;  // インスタンスカラー（オプション）
    };

    /// @brief インスタンス描画用バッチ
    struct RenderBatch {
        Mesh*       m_mesh     = nullptr;
        Material*   m_material = nullptr;
        std::vector<InstanceData> m_instances;
    };

} // namespace Engine
```

```cpp
// Engine/Graphics/RenderSystem.h に追加
class RenderSystem final {
public:
    /// @brief インスタンス描画用にバッチを構築
    void BuildBatches();

private:
    void DrawBatch(ID3D11DeviceContext* context, const RenderBatch& batch);
    void CreateInstanceBuffer(size_t maxInstances);

private:
    std::vector<RenderBatch> m_batches;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_instanceBuffer;
    size_t m_maxInstanceCount = 1024;
};
```

```cpp
// RenderSystem.cpp
void RenderSystem::BuildBatches() {
    m_batches.clear();

    // キー: Mesh* + Material*
    std::unordered_map<std::pair<Mesh*, Material*>, RenderBatch, PairHash> batchMap;

    for (const auto& item : m_items) {
        auto key = std::make_pair(item.m_mesh, item.m_material);
        auto& batch = batchMap[key];

        if (batch.m_mesh == nullptr) {
            batch.m_mesh = item.m_mesh;
            batch.m_material = item.m_material;
        }

        InstanceData inst;
        inst.m_world = item.m_world;
        inst.m_color = { 1.0f, 1.0f, 1.0f, 1.0f };
        batch.m_instances.push_back(inst);
    }

    for (auto& [key, batch] : batchMap) {
        if (batch.m_instances.size() > 1) {
            // インスタンス描画
            m_batches.push_back(std::move(batch));
        }
    }
}

void RenderSystem::DrawBatch(ID3D11DeviceContext* context, const RenderBatch& batch) {
    // インスタンスバッファ更新
    D3D11_MAPPED_SUBRESOURCE mapped;
    context->Map(m_instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, batch.m_instances.data(), 
           batch.m_instances.size() * sizeof(InstanceData));
    context->Unmap(m_instanceBuffer.Get(), 0);

    // バッファ設定
    ID3D11Buffer* buffers[] = { batch.m_mesh->GetVertexBuffer(), m_instanceBuffer.Get() };
    UINT strides[] = { batch.m_mesh->GetStride(), sizeof(InstanceData) };
    UINT offsets[] = { 0, 0 };
    context->IASetVertexBuffers(0, 2, buffers, strides, offsets);

    // インスタンス描画
    context->DrawIndexedInstanced(
        batch.m_mesh->GetIndexCount(),
        static_cast<UINT>(batch.m_instances.size()),
        0, 0, 0
    );
}
```

---

### 3. 定数バッファ最適化（中優先度）

**目的:** 不要な更新を削減

```cpp
// Engine/Graphics/ConstantBuffer.h に追加
template<typename T>
class ConstantBuffer final {
public:
    /// @brief データが変更された場合のみ更新
    void UpdateIfDirty(ID3D11DeviceContext* context) {
        if (!m_isDirty) return;

        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(context->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            memcpy(mapped.pData, &m_data, sizeof(T));
            context->Unmap(m_buffer.Get(), 0);
            m_isDirty = false;
        }
    }

    /// @brief データを設定（変更があれば dirty フラグを立てる）
    void SetData(const T& data) {
        if (memcmp(&m_data, &data, sizeof(T)) != 0) {
            m_data = data;
            m_isDirty = true;
        }
    }

    /// @brief 強制的に dirty フラグを立てる
    void MarkDirty() {
        m_isDirty = true;
    }

private:
    T m_data{};
    bool m_isDirty = true;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;
};
```

---

### 4. ステート管理の最適化（中優先度）

**目的:** 冗長なステート設定を回避

```cpp
// Engine/Graphics/StateCache.h
#pragma once

#include <d3d11.h>

namespace Engine {

    /// @brief D3D11ステートのキャッシュ
    class StateCache final {
    public:
        StateCache() = default;
        ~StateCache() = default;

        StateCache(const StateCache&) = delete;
        StateCache& operator=(const StateCache&) = delete;

        void Initialize(ID3D11DeviceContext* context) {
            m_context = context;
            Reset();
        }

        /// @brief 深度ステンシルステートを設定（変更時のみ）
        void SetDepthStencilState(ID3D11DepthStencilState* state, UINT stencilRef = 0) {
            if (m_currentDS != state || m_currentStencilRef != stencilRef) {
                m_context->OMSetDepthStencilState(state, stencilRef);
                m_currentDS = state;
                m_currentStencilRef = stencilRef;
            }
        }

        /// @brief ラスタライザステートを設定（変更時のみ）
        void SetRasterizerState(ID3D11RasterizerState* state) {
            if (m_currentRS != state) {
                m_context->RSSetState(state);
                m_currentRS = state;
            }
        }

        /// @brief ブレンドステートを設定（変更時のみ）
        void SetBlendState(ID3D11BlendState* state, const float* blendFactor = nullptr) {
            if (m_currentBS != state) {
                m_context->OMSetBlendState(state, blendFactor, 0xFFFFFFFF);
                m_currentBS = state;
            }
        }

        /// @brief 頂点シェーダーを設定（変更時のみ）
        void SetVertexShader(ID3D11VertexShader* shader) {
            if (m_currentVS != shader) {
                m_context->VSSetShader(shader, nullptr, 0);
                m_currentVS = shader;
            }
        }

        /// @brief ピクセルシェーダーを設定（変更時のみ）
        void SetPixelShader(ID3D11PixelShader* shader) {
            if (m_currentPS != shader) {
                m_context->PSSetShader(shader, nullptr, 0);
                m_currentPS = shader;
            }
        }

        /// @brief 入力レイアウトを設定（変更時のみ）
        void SetInputLayout(ID3D11InputLayout* layout) {
            if (m_currentIL != layout) {
                m_context->IASetInputLayout(layout);
                m_currentIL = layout;
            }
        }

        /// @brief キャッシュをリセット（フレーム開始時など）
        void Reset() {
            m_currentDS = nullptr;
            m_currentRS = nullptr;
            m_currentBS = nullptr;
            m_currentVS = nullptr;
            m_currentPS = nullptr;
            m_currentIL = nullptr;
            m_currentStencilRef = 0;
        }

    private:
        ID3D11DeviceContext*     m_context = nullptr;
        ID3D11DepthStencilState* m_currentDS = nullptr;
        ID3D11RasterizerState*   m_currentRS = nullptr;
        ID3D11BlendState*        m_currentBS = nullptr;
        ID3D11VertexShader*      m_currentVS = nullptr;
        ID3D11PixelShader*       m_currentPS = nullptr;
        ID3D11InputLayout*       m_currentIL = nullptr;
        UINT                     m_currentStencilRef = 0;
    };

} // namespace Engine
```

---

### 5. テクスチャアトラス対応（低優先度）

**目的:** テクスチャ切替を削減

```cpp
// Engine/Graphics/TextureAtlas.h
#pragma once

#include <DirectXMath.h>
#include <unordered_map>
#include <string>

namespace Engine {

    /// @brief テクスチャアトラス内の領域
    struct AtlasRegion {
        float u0, v0;  // 左上UV
        float u1, v1;  // 右下UV
        int   width;   // ピクセル幅
        int   height;  // ピクセル高
    };

    /// @brief テクスチャアトラス管理
    class TextureAtlas final {
    public:
        TextureAtlas() = default;
        ~TextureAtlas() = default;

        bool Load(ID3D11Device* device, const std::wstring& atlasPath, const std::wstring& metaPath);
        void Unload();

        /// @brief 名前で領域を取得
        [[nodiscard]] const AtlasRegion* GetRegion(const std::string& name) const;

        /// @brief テクスチャを取得
        [[nodiscard]] ID3D11ShaderResourceView* GetSRV() const;

    private:
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;
        std::unordered_map<std::string, AtlasRegion> m_regions;
    };

} // namespace Engine
```

---

### 6. リソースプール（中優先度）

**目的:** 動的リソースの再利用でアロケーション削減

```cpp
// Engine/Graphics/BufferPool.h
#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <queue>

namespace Engine {

    /// @brief 動的バッファのプール
    class DynamicBufferPool final {
    public:
        DynamicBufferPool() = default;
        ~DynamicBufferPool() = default;

        DynamicBufferPool(const DynamicBufferPool&) = delete;
        DynamicBufferPool& operator=(const DynamicBufferPool&) = delete;

        void Initialize(ID3D11Device* device, size_t bufferSize, size_t poolSize);
        void Finalize();

        /// @brief バッファを取得（プールから借用）
        ID3D11Buffer* Acquire();

        /// @brief バッファを返却（フレーム終了時）
        void Release(ID3D11Buffer* buffer);

        /// @brief フレーム終了時にリセット
        void ResetFrame();

    private:
        ID3D11Device* m_device = nullptr;
        size_t m_bufferSize = 0;

        std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> m_pool;
        std::queue<ID3D11Buffer*> m_available;
        std::vector<ID3D11Buffer*> m_inUse;
    };

} // namespace Engine
```

---

## パフォーマンス計測

### 推奨プロファイリングポイント

```cpp
// Engine/Core/Profiler.h
#pragma once

#include <chrono>
#include <string>
#include <unordered_map>

namespace Engine {

    class Profiler final {
    public:
        static void BeginFrame();
        static void EndFrame();

        static void BeginSection(const std::string& name);
        static void EndSection(const std::string& name);

        static float GetSectionTimeMs(const std::string& name);
        static float GetFrameTimeMs();

    private:
        struct SectionData {
            std::chrono::high_resolution_clock::time_point startTime;
            float durationMs = 0.0f;
        };

        static inline std::unordered_map<std::string, SectionData> s_sections;
        static inline std::chrono::high_resolution_clock::time_point s_frameStart;
        static inline float s_frameTimeMs = 0.0f;
    };

    /// @brief スコープベースのプロファイラー
    class ScopedProfile final {
    public:
        explicit ScopedProfile(const std::string& name) : m_name(name) {
            Profiler::BeginSection(m_name);
        }
        ~ScopedProfile() {
            Profiler::EndSection(m_name);
        }

    private:
        std::string m_name;
    };

} // namespace Engine

// 使用例
#define PROFILE_SCOPE(name) Engine::ScopedProfile _profile##__LINE__(name)

void RenderSystem::Draw(World& world) {
    PROFILE_SCOPE("RenderSystem::Draw");
    // ...
}
```

---

## 最適化の優先順位

| 順序 | 最適化項目 | 優先度 | 効果 | 工数 |
|------|-----------|--------|------|------|
| 1 | RenderItemソート | 高 | シェーダー切替削減 | 2日 |
| 2 | ステートキャッシュ | 高 | ステート設定削減 | 1日 |
| 3 | 定数バッファ最適化 | 中 | Map/Unmap削減 | 1日 |
| 4 | インスタンシング | 高 | DrawCall削減 | 1週間 |
| 5 | プロファイラー | 中 | 計測基盤 | 2日 |
| 6 | リソースプール | 中 | アロケーション削減 | 3日 |
| 7 | テクスチャアトラス | 低 | テクスチャ切替削減 | 3日 |

---

## 期待される効果

| 最適化項目 | 期待効果 |
|-----------|---------|
| ソート最適化 | シェーダー切替 50-80% 削減 |
| ステートキャッシュ | ステート設定 30-50% 削減 |
| インスタンシング | DrawCall 大幅削減（同一オブジェクト数依存） |
| 定数バッファ | CPU→GPU転送 20-40% 削減 |

---

## 次のステップ

1. [ROADMAP.md](./ROADMAP.md) で全体計画を確認
2. プロファイラーを先に実装して計測基盤を整備
3. ソート最適化とステートキャッシュを実装
4. 効果を計測して次の最適化を検討
