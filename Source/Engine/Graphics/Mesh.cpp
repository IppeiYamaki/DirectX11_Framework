#include "Mesh.h"

#include "Engine/Core/Logger.h"
#include "Engine/Core/Assert.h"

namespace Engine {

    Mesh::~Mesh() {
        Destroy();
    }

    bool Mesh::Create(
        ID3D11Device* device,
        const void* vertexData,
        std::uint32_t vertexStride,
        std::uint32_t vertexCount,
        const std::uint32_t* indexData,
        std::uint32_t indexCount
    ) {
        if (device == nullptr) {
            Logger::Error("Mesh::Create failed: device is null.");
            return false;
        }
        if (vertexData == nullptr || vertexStride == 0 || vertexCount == 0) {
            Logger::Error("Mesh::Create failed: vertex is invalid.");
            return false;
        }

        Destroy();

        m_vertexStride = vertexStride;
        m_vertexCount = vertexCount;
        m_indexCount = indexCount;

        // VertexBuffer
        {
            D3D11_BUFFER_DESC desc{};
            desc.Usage = D3D11_USAGE_IMMUTABLE;
            desc.ByteWidth = vertexStride * vertexCount;
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

            D3D11_SUBRESOURCE_DATA init{};
            init.pSysMem = vertexData;

            const HRESULT hr = device->CreateBuffer(&desc, &init, m_vertexBuffer.GetAddressOf());
            if (FAILED(hr)) {
                Engine::Assert::ReportHrFailure(hr, "CreateBuffer(VertexBuffer)", __FILE__, __LINE__, __func__);
                Destroy();
                return false;
            }
        }

        // IndexBufferi”CˆÓj
        if (indexData != nullptr && indexCount > 0) {
            D3D11_BUFFER_DESC desc{};
            desc.Usage = D3D11_USAGE_IMMUTABLE;
            desc.ByteWidth = static_cast<UINT>(sizeof(std::uint32_t) * indexCount);
            desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

            D3D11_SUBRESOURCE_DATA init{};
            init.pSysMem = indexData;

            const HRESULT hr = device->CreateBuffer(&desc, &init, m_indexBuffer.GetAddressOf());
            if (FAILED(hr)) {
                Engine::Assert::ReportHrFailure(hr, "CreateBuffer(IndexBuffer)", __FILE__, __LINE__, __func__);
                Destroy();
                return false;
            }
        }

        return true;
    }

    void Mesh::Destroy() {
        m_indexBuffer.Reset();
        m_vertexBuffer.Reset();

        m_vertexStride = 0;
        m_vertexCount = 0;
        m_indexCount = 0;
    }

    void Mesh::Bind(ID3D11DeviceContext* context) const {
        if (context == nullptr) return;
        if (!m_vertexBuffer) return;

        const UINT stride = m_vertexStride;
        const UINT offset = 0;
        ID3D11Buffer* vb = m_vertexBuffer.Get();
        context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

        if (m_indexBuffer) {
            context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        }
    }

    void Mesh::Draw(ID3D11DeviceContext* context) const {
        if (context == nullptr) return;
        if (!m_vertexBuffer) return;

        if (m_indexBuffer && m_indexCount > 0) {
            context->DrawIndexed(m_indexCount, 0, 0);
        }
        else {
            context->Draw(m_vertexCount, 0);
        }
    }

    bool Mesh::IsValid() const {
        return (m_vertexBuffer != nullptr) && (m_vertexCount > 0) && (m_vertexStride > 0);
    }

    std::uint32_t Mesh::GetVertexCount() const { return m_vertexCount; }
    std::uint32_t Mesh::GetIndexCount() const { return m_indexCount; }
    std::uint32_t Mesh::GetVertexStride() const { return m_vertexStride; }

} // namespace Engine
