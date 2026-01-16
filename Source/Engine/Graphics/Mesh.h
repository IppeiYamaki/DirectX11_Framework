#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <cstdint>

namespace Engine {

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

        // Subset•`‰æ—p
        void DrawRange(
            ID3D11DeviceContext* context,
            std::uint32_t indexCount,
            std::uint32_t startIndex = 0,
            std::int32_t baseVertex = 0
        ) const;

        bool IsValid() const;

        std::uint32_t GetVertexCount() const;
        std::uint32_t GetIndexCount() const;
        std::uint32_t GetVertexStride() const;

    private:
        std::uint32_t m_vertexStride = 0;
        std::uint32_t m_vertexCount = 0;
        std::uint32_t m_indexCount = 0;

        Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
    };

} // namespace Engine
