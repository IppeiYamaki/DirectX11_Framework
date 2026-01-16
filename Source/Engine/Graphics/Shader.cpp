#include "Shader.h"

#include "Engine/Core/Logger.h"
#include "Engine/Core/Assert.h"

#include <fstream>
#include <iterator>

namespace Engine {

    //============================================================
    // Helpers
    //============================================================
    static bool ReadFileBinary(const std::wstring& path, std::vector<std::uint8_t>& outData) {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            Logger::Error("ReadFileBinary failed: file not found.");

            std::string s(path.begin(), path.end());
            Engine::Logger::Error(("Tried to open: " + s).c_str());

            return false;
        }

        file.seekg(0, std::ios::end);
        const std::streamoff size = file.tellg();
        if (size <= 0) {
            Logger::Error("ReadFileBinary failed: file size is 0.");
            return false;
        }
        file.seekg(0, std::ios::beg);

        outData.resize(static_cast<size_t>(size));
        file.read(reinterpret_cast<char*>(outData.data()), size);
        return true;
    }

    VertexInputLayout CreateDefaultPosNormColorUvLayout() {
        VertexInputLayout layout;
        layout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layout.push_back({ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layout.push_back({ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 });
        layout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 });
        return layout;
    }

    //============================================================
    // VertexShader
    //============================================================
    VertexShader::~VertexShader() {
        Unload();
    }

    bool VertexShader::LoadFromCSO(ID3D11Device* device, const std::wstring& csoPath) {
        if (device == nullptr) {
            Logger::Error("VertexShader::LoadFromCSO failed: device is null.");
            return false;
        }

        Unload();
        m_path = csoPath;

        if (!ReadFileBinary(csoPath, m_bytecode)) {
            Logger::Error("VertexShader::LoadFromCSO failed: read file failed.");
            Unload();
            return false;
        }

        HRESULT hr = device->CreateVertexShader(
            m_bytecode.data(),
            m_bytecode.size(),
            nullptr,
            m_shader.GetAddressOf()
        );

        if (FAILED(hr)) {
            Engine::Assert::ReportHrFailure(hr, "CreateVertexShader", __FILE__, __LINE__, __func__);
            Unload();
            return false;
        }

        return true;
    }

    void VertexShader::Unload() {
        m_shader.Reset();
        m_bytecode.clear();
        m_path.clear();
    }

    bool VertexShader::IsLoaded() const {
        return (m_shader != nullptr);
    }

    ID3D11VertexShader* VertexShader::GetShader() const {
        return m_shader.Get();
    }

    const void* VertexShader::GetBytecodeData() const {
        return m_bytecode.empty() ? nullptr : m_bytecode.data();
    }

    size_t VertexShader::GetBytecodeSize() const {
        return m_bytecode.size();
    }

    const std::wstring& VertexShader::GetPath() const {
        return m_path;
    }

    //============================================================
    // PixelShader
    //============================================================
    PixelShader::~PixelShader() {
        Unload();
    }

    bool PixelShader::LoadFromCSO(ID3D11Device* device, const std::wstring& csoPath) {
        if (device == nullptr) {
            Logger::Error("PixelShader::LoadFromCSO failed: device is null.");
            return false;
        }

        Unload();
        m_path = csoPath;

        std::vector<std::uint8_t> bytecode;
        if (!ReadFileBinary(csoPath, bytecode)) {
            Logger::Error("PixelShader::LoadFromCSO failed: read file failed.");
            Unload();
            return false;
        }

        HRESULT hr = device->CreatePixelShader(
            bytecode.data(),
            bytecode.size(),
            nullptr,
            m_shader.GetAddressOf()
        );

        if (FAILED(hr)) {
            Engine::Assert::ReportHrFailure(hr, "CreatePixelShader", __FILE__, __LINE__, __func__);
            Unload();
            return false;
        }

        return true;
    }

    void PixelShader::Unload() {
        m_shader.Reset();
        m_path.clear();
    }

    bool PixelShader::IsLoaded() const {
        return (m_shader != nullptr);
    }

    ID3D11PixelShader* PixelShader::GetShader() const {
        return m_shader.Get();
    }

    const std::wstring& PixelShader::GetPath() const {
        return m_path;
    }

    //============================================================
    // InputLayout
    //============================================================
    InputLayout::~InputLayout() {
        Unload();
    }

    bool InputLayout::Create(
        ID3D11Device* device,
        const VertexInputLayout& layout,
        const void* vsBytecode,
        size_t vsBytecodeSize
    ) {
        if (device == nullptr) {
            Logger::Error("InputLayout::Create failed: device is null.");
            return false;
        }
        if (vsBytecode == nullptr || vsBytecodeSize == 0) {
            Logger::Error("InputLayout::Create failed: vs bytecode is invalid.");
            return false;
        }

        Unload();

        // D3D11_INPUT_ELEMENT_DESC ÇàÍéûç\ízÅiSemanticNameÇÃéıñΩÇ layout ÇÃ string Ç™ï€èÿÅj
        std::vector<D3D11_INPUT_ELEMENT_DESC> descs;
        descs.reserve(layout.size());

        for (const auto& e : layout) {
            D3D11_INPUT_ELEMENT_DESC d{};
            d.SemanticName = e.m_semanticName.c_str();
            d.SemanticIndex = e.m_semanticIndex;
            d.Format = e.m_format;
            d.InputSlot = e.m_inputSlot;
            d.AlignedByteOffset = e.m_alignedByteOffset;
            d.InputSlotClass = e.m_inputSlotClass;
            d.InstanceDataStepRate = e.m_instanceDataStepRate;
            descs.push_back(d);
        }

        HRESULT hr = device->CreateInputLayout(
            descs.data(),
            static_cast<UINT>(descs.size()),
            vsBytecode,
            vsBytecodeSize,
            m_inputLayout.GetAddressOf()
        );

        if (FAILED(hr)) {
            Engine::Assert::ReportHrFailure(hr, "CreateInputLayout", __FILE__, __LINE__, __func__);
            Unload();
            return false;
        }

        return true;
    }

    void InputLayout::Unload() {
        m_inputLayout.Reset();
    }

    bool InputLayout::IsCreated() const {
        return (m_inputLayout != nullptr);
    }

    ID3D11InputLayout* InputLayout::GetInputLayout() const {
        return m_inputLayout.Get();
    }

} // namespace Engine
