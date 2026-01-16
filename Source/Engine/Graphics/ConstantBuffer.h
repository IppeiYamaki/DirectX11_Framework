#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "Engine/Core/Logger.h"
#include "Engine/Core/Assert.h"

namespace Engine {

    /**
     * @brief D3D11 定数バッファ（Dynamic / WriteDiscard）
     *
     * - Create() で作成
     * - Update() で毎フレーム更新
     * - GetBuffer() で借用ポインタ取得
     */
    template <class T>
    class ConstantBuffer final {
    public:
        ConstantBuffer() = default;
        ~ConstantBuffer() { Destroy(); }

        ConstantBuffer(const ConstantBuffer&) = delete;
        ConstantBuffer& operator=(const ConstantBuffer&) = delete;

        bool Create(ID3D11Device* device) {
            if (device == nullptr) {
                Logger::Error("ConstantBuffer::Create failed: device is null.");
                return false;
            }

            Destroy();

            D3D11_BUFFER_DESC desc{};
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.ByteWidth = static_cast<UINT>((sizeof(T) + 15) / 16 * 16); // 16B丸め
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            const HRESULT hr = device->CreateBuffer(&desc, nullptr, m_buffer.GetAddressOf());
            if (FAILED(hr)) {
                Engine::Assert::ReportHrFailure(hr, "CreateBuffer(ConstantBuffer)", __FILE__, __LINE__, __func__);
                Destroy();
                return false;
            }
            return true;
        }

        void Destroy() {
            m_buffer.Reset();
        }

        bool Update(ID3D11DeviceContext* context, const T& data) {
            if (context == nullptr || m_buffer == nullptr) return false;

            D3D11_MAPPED_SUBRESOURCE mapped{};
            const HRESULT hr = context->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            if (FAILED(hr)) {
                Engine::Assert::ReportHrFailure(hr, "Map(ConstantBuffer)", __FILE__, __LINE__, __func__);
                return false;
            }

            std::memcpy(mapped.pData, &data, sizeof(T));
            context->Unmap(m_buffer.Get(), 0);
            return true;
        }

        bool IsValid() const { return m_buffer != nullptr; }

        ID3D11Buffer* GetBuffer() const { return m_buffer.Get(); }

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;
    };

} // namespace Engine
