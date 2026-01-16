#pragma once

#include <d3d11.h>

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "Engine/Graphics/Texture.h"
#include "Engine/Graphics/ShaderLibrary.h"

namespace Engine {

    class Model;

    /**
     * @brief アセット読み込み窓口の統一（キャッシュ付き）
     *
     * - 同一キー（パス + オプション）なら同一インスタンスを返す
     * - Texture / Shader / Model をここからロードする
     */
    class AssetManager final {
    public:
        AssetManager() = default;
        ~AssetManager();

        AssetManager(const AssetManager&) = delete;
        AssetManager& operator=(const AssetManager&) = delete;

        //============================================================
        // Lifecycle
        //============================================================
        bool Initialize(ID3D11Device* device);
        void Finalize();
        void Reset(); // キャッシュ全消去

        bool IsInitialized() const;

        //============================================================
        // Base directory (optional)
        //============================================================
        void SetBaseDirectory(const std::wstring& baseDir);
        const std::wstring& GetBaseDirectory() const;

        //============================================================
        // Texture
        //============================================================
        std::shared_ptr<Texture> LoadTexture(
            const std::wstring& path,
            const TextureLoadOptions& options = {}
        );

        void PruneUnusedTextures();

        //============================================================
        // Shader
        //============================================================
        std::shared_ptr<VertexShader> LoadVertexShader(const std::wstring& key, const std::wstring& csoPath);
        std::shared_ptr<PixelShader> LoadPixelShader(const std::wstring& key, const std::wstring& csoPath);

        std::shared_ptr<VertexShader> LoadVertexShader(const std::wstring& csoPath);
        std::shared_ptr<PixelShader> LoadPixelShader(const std::wstring& csoPath);

        std::shared_ptr<InputLayout> CreateInputLayout(
            const std::wstring& layoutKey,
            const VertexInputLayout& layout,
            const VertexShader& vertexShader
        );

        ShaderLibrary* GetShaderLibrary();
        const ShaderLibrary* GetShaderLibrary() const;

        //============================================================
        // Model（現状は OBJ のみ対応）
        //============================================================
        std::shared_ptr<Model> LoadModel(const std::wstring& path);
        void PruneUnusedModels();

        ID3D11Device* GetDevice() const;

    private:
        std::wstring ResolvePath(const std::wstring& path) const;
        std::wstring MakeTextureCacheKey(const std::wstring& resolvedPath, const TextureLoadOptions& options) const;
        std::wstring MakeModelCacheKey(const std::wstring& resolvedPath) const;

    private:
        bool m_isInitialized = false;

        ID3D11Device* m_device = nullptr;

        std::wstring m_baseDirectory = L"";

        ShaderLibrary m_shaderLibrary;

        std::unordered_map<std::wstring, std::weak_ptr<Texture>> m_textureCache;
        std::unordered_map<std::wstring, std::weak_ptr<Model>>   m_modelCache;

        mutable std::mutex m_mutex;
    };

} // namespace Engine
