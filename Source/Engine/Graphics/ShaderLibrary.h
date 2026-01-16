#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "Engine/Graphics/Shader.h"

namespace Engine {

    /**
     * @brief Shaderのキャッシュ管理（あなたのShaderManagerに相当）
     *
     * - Loadはキャッシュして同一キーは同一インスタンスを返す
     * - 入力レイアウトは (layoutKey) でキャッシュ
     */
    class ShaderLibrary final {
    public:
        ShaderLibrary() = default;
        ~ShaderLibrary();

        ShaderLibrary(const ShaderLibrary&) = delete;
        ShaderLibrary& operator=(const ShaderLibrary&) = delete;

        bool Initialize(ID3D11Device* device);
        void Finalize();
        void Reset();

        std::shared_ptr<VertexShader> LoadVertexShader(const std::wstring& key, const std::wstring& csoPath);
        std::shared_ptr<PixelShader> LoadPixelShader(const std::wstring& key, const std::wstring& csoPath);

        std::shared_ptr<InputLayout> CreateInputLayout(
            const std::wstring& layoutKey,
            const VertexInputLayout& layout,
            const VertexShader& vertexShader
        );

        ID3D11Device* GetDevice() const;

    private:
        ID3D11Device* m_device = nullptr; // 借用

        std::unordered_map<std::wstring, std::shared_ptr<VertexShader>> m_vsCache;
        std::unordered_map<std::wstring, std::shared_ptr<PixelShader>>  m_psCache;
        std::unordered_map<std::wstring, std::shared_ptr<InputLayout>>  m_layoutCache;

        bool m_isInitialized = false;
    };

} // namespace Engine
