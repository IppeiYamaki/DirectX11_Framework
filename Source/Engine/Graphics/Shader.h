#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <cstdint>
#include <string>
#include <vector>

namespace Engine {

    //============================================================
    // Vertex Input Layout (安全に保持できる表現)
    //============================================================
    struct VertexInputElement final {
        std::string m_semanticName;                 // "POSITION" etc
        UINT m_semanticIndex = 0;
        DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;
        UINT m_inputSlot = 0;
        UINT m_alignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        D3D11_INPUT_CLASSIFICATION m_inputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        UINT m_instanceDataStepRate = 0;
    };

    using VertexInputLayout = std::vector<VertexInputElement>;

    // よく使うレイアウト（あなたの例に合わせた便利関数）
    VertexInputLayout CreateDefaultPosNormColorUvLayout();

    //============================================================
    // VertexShader
    //============================================================
    class VertexShader final {
    public:
        VertexShader() = default;
        ~VertexShader();

        VertexShader(const VertexShader&) = delete;
        VertexShader& operator=(const VertexShader&) = delete;

        bool LoadFromCSO(ID3D11Device* device, const std::wstring& csoPath);
        void Unload();

        bool IsLoaded() const;

        ID3D11VertexShader* GetShader() const;

        const void*         GetBytecodeData() const;
        size_t              GetBytecodeSize() const;

        const std::wstring& GetPath() const;

    private:
		std::wstring                                m_path{};   // CSOファイルパス
		std::vector<std::uint8_t>                   m_bytecode; // バイトコード
		Microsoft::WRL::ComPtr<ID3D11VertexShader>  m_shader;   // シェーダ
    };

    //============================================================
    // PixelShader
    //============================================================
    class PixelShader final {
    public:
        PixelShader() = default;
        ~PixelShader();

        PixelShader(const PixelShader&) = delete;
        PixelShader& operator=(const PixelShader&) = delete;

        bool LoadFromCSO(ID3D11Device* device, const std::wstring& csoPath);
        void Unload();

        bool IsLoaded() const;

        ID3D11PixelShader*  GetShader() const;

        const std::wstring& GetPath() const;

    private:
		std::wstring                                m_path{};   // CSOファイルパス
		Microsoft::WRL::ComPtr<ID3D11PixelShader>   m_shader;   // シェーダ
    };

    //============================================================
    // InputLayout
    //============================================================
    class InputLayout final {
    public:
        InputLayout() = default;
        ~InputLayout();

        InputLayout(const InputLayout&) = delete;
        InputLayout& operator=(const InputLayout&) = delete;

        bool Create(
            ID3D11Device* device,
            const VertexInputLayout& layout,
            const void* vsBytecode,
            size_t vsBytecodeSize
        );

        void Unload();
        bool IsCreated() const;

        ID3D11InputLayout* GetInputLayout() const;

    private:
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;    // 入力レイアウト
    };

} // namespace Engine
