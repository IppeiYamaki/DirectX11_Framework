#include "ShaderLibrary.h"

#include "Engine/Core/Logger.h"

namespace Engine {

    ShaderLibrary::~ShaderLibrary() {
        Finalize();
    }

    bool ShaderLibrary::Initialize(ID3D11Device* device) {
        if (m_isInitialized) return true;

        if (device == nullptr) {
            Logger::Error("ShaderLibrary::Initialize failed: device is null.");
            return false;
        }

        m_device = device;
        m_isInitialized = true;
        return true;
    }

    void ShaderLibrary::Finalize() {
        m_layoutCache.clear();
        m_vsCache.clear();
        m_psCache.clear();

        m_device = nullptr;
        m_isInitialized = false;
    }

    void ShaderLibrary::Reset() {
        // キャッシュをクリアしたいならここで
        m_layoutCache.clear();
        m_vsCache.clear();
        m_psCache.clear();
    }

    std::shared_ptr<VertexShader> ShaderLibrary::LoadVertexShader(const std::wstring& key, const std::wstring& csoPath) {
        auto it = m_vsCache.find(key);
        if (it != m_vsCache.end()) return it->second;

        auto shader = std::make_shared<VertexShader>();
        if (!shader->LoadFromCSO(m_device, csoPath)) {
            Logger::Error("ShaderLibrary: LoadVertexShader failed.");
            return nullptr;
        }

        m_vsCache.emplace(key, shader);
        return shader;
    }

    std::shared_ptr<PixelShader> ShaderLibrary::LoadPixelShader(const std::wstring& key, const std::wstring& csoPath) {
        auto it = m_psCache.find(key);
        if (it != m_psCache.end()) return it->second;

        auto shader = std::make_shared<PixelShader>();
        if (!shader->LoadFromCSO(m_device, csoPath)) {
            Logger::Error("ShaderLibrary: LoadPixelShader failed.");
            return nullptr;
        }

        m_psCache.emplace(key, shader);
        return shader;
    }

    std::shared_ptr<InputLayout> ShaderLibrary::CreateInputLayout(
        const std::wstring& layoutKey,
        const VertexInputLayout& layout,
        const VertexShader& vertexShader
    ) {
        auto it = m_layoutCache.find(layoutKey);
        if (it != m_layoutCache.end()) return it->second;

        auto il = std::make_shared<InputLayout>();
        if (!il->Create(m_device, layout, vertexShader.GetBytecodeData(), vertexShader.GetBytecodeSize())) {
            Logger::Error("ShaderLibrary: CreateInputLayout failed.");
            return nullptr;
        }

        m_layoutCache.emplace(layoutKey, il);
        return il;
    }

    ID3D11Device* ShaderLibrary::GetDevice() const {
        return m_device;
    }

} // namespace Engine
