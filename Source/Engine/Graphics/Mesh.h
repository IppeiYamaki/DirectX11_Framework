#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <cstdint>

namespace Engine {

    /**
     * @brief 頂点/インデックスバッファの薄いラッパ
     *
     * - Create() でGPUバッファ生成
     * - Bind() でIAへセット
     * - Draw() で描画
     */
    class Mesh final {
    public:
        Mesh() = default;
        ~Mesh();

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        bool Create(
            ID3D11Device* device,
            const void* vertexData,
            std::uint32_t vertexStride,
            std::uint32_t vertexCount,
            const std::uint32_t* indexData,
            std::uint32_t indexCount
        );

        void Destroy();

        void Bind(ID3D11DeviceContext* context) const;
        void Draw(ID3D11DeviceContext* context) const;

        bool IsValid() const;

        std::uint32_t GetVertexCount() const;
        std::uint32_t GetIndexCount() const;
        std::uint32_t GetVertexStride() const;

    private:
        std::uint32_t                           m_vertexStride  = 0;    // バイト単位
        std::uint32_t                           m_vertexCount   = 0;    // 頂点数
        std::uint32_t                           m_indexCount    = 0;    // インデックス数

		Microsoft::WRL::ComPtr<ID3D11Buffer>    m_vertexBuffer;         // 頂点バッファ
		Microsoft::WRL::ComPtr<ID3D11Buffer>    m_indexBuffer;          // インデックスバッファ
    };

} // namespace Engine
