#pragma once

#include <d3d11.h>

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "Engine/Graphics/Texture.h"
#include "Engine/Graphics/ShaderLibrary.h"

namespace Engine {

    /**
     * @brief アセット読み込み入口の統一（キャッシュ付き）
     *
     * - 同一キー（パス + オプション）なら同一インスタンスを返す
     * - Texture / Shader をここからロードする
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

        // 使われなくなったTextureキャッシュを掃除（weak_ptrが切れてるものを消す）
        void PruneUnusedTextures();

        //============================================================
        // Shader
        //============================================================
        // key と path を分けて渡したい版（あなたのShaderManagerに近い）
        std::shared_ptr<VertexShader> LoadVertexShader(const std::wstring& key, const std::wstring& csoPath);
        std::shared_ptr<PixelShader> LoadPixelShader(const std::wstring& key, const std::wstring& csoPath);

        // path=key で簡単に呼べる版（おすすめ）
        std::shared_ptr<VertexShader> LoadVertexShader(const std::wstring& csoPath);
        std::shared_ptr<PixelShader> LoadPixelShader(const std::wstring& csoPath);

        // InputLayout（layoutKeyでキャッシュ）
        std::shared_ptr<InputLayout> CreateInputLayout(
            const std::wstring& layoutKey,
            const VertexInputLayout& layout,
            const VertexShader& vertexShader
        );

        // 低レベルに触りたいとき用
              ShaderLibrary* GetShaderLibrary();
        const ShaderLibrary* GetShaderLibrary() const;

        ID3D11Device* GetDevice() const;

    private:
        std::wstring ResolvePath(const std::wstring& path) const;
        std::wstring MakeTextureCacheKey(const std::wstring& resolvedPath, const TextureLoadOptions& options) const;

    private:
		bool                                                        m_isInitialized = false;    // 初期化済みフラグ

        ID3D11Device*                                               m_device        = nullptr;  // D3D11デバイス（借用）

		std::wstring                                                m_baseDirectory = L"";      // ベースディレクトリ

		ShaderLibrary                                               m_shaderLibrary;            // Shaderライブラリ

        // Texture cache（オプション込みのキー → weak_ptr）
		std::unordered_map<std::wstring, std::weak_ptr<Texture>>    m_textureCache;             // キャッシュ

		mutable std::mutex                                          m_mutex;                    // スレッドセーフ用ミューテックス
    };

} // namespace Engine
