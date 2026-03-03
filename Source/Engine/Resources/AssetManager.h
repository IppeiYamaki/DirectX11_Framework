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

    /// @brief テクスチャやシェーダー、モデルなどのアセットを管理するクラス
	/// @note  内部でキャッシュを持ち、同じパスのアセットは再利用する。不要になったアセットは PruneUnused*() でキャッシュから削除できる。
    class AssetManager final {
    public:
        AssetManager() = default;
        ~AssetManager();

        AssetManager(const AssetManager&) = delete;
        AssetManager& operator=(const AssetManager&) = delete;

        //============================================================
        // Lifecycle
        //============================================================
		/// @brief  初期化
		/// @param  device D3D11デバイス
        /// @return 成功した場合は true
        bool Initialize(ID3D11Device* device);
		/// @brief  終了処理
        void Finalize();
		/// @brief  リセット（キャッシュクリア）
        void Reset();

		/// @brief  初期化済みか確認
		/// @return 初期化済みなら true
        bool IsInitialized() const;

        //============================================================
        // Base directory (optional)
        //============================================================
		/// @brief  ベースディレクトリを設定。相対パスでアセットを指定する場合、このディレクトリが基準になる。
		/// @param  baseDir ベースディレクトリのパス（ワイド文字列）
        void SetBaseDirectory(const std::wstring& baseDir);
		/// @brief  ベースディレクトリを取得
		/// @return ベースディレクトリのパス（ワイド文字列）
        const std::wstring& GetBaseDirectory() const;

        //============================================================
        // Texture
        //============================================================
		/// @brief  テクスチャを読み込む。すでに同じパスとオプションで読み込まれているテクスチャがあれば、それを再利用する。
		/// @param  path テクスチャファイルのパス（ワイド文字列）。ベースディレクトリからの相対パスも可。
		/// @param  options 読み込みオプション（省略可）
		/// @return 読み込まれたテクスチャ（失敗した場合は nullptr）
        std::shared_ptr<Texture> LoadTexture(
            const std::wstring& path,
            const TextureLoadOptions& options = {}
        );

		/// @brief  使用されていないテクスチャをキャッシュから削除する。共有ポインタが切れている（参照カウントが0の）テクスチャが対象。
        void PruneUnusedTextures();

        //============================================================
        // Shader
        //============================================================
		/// @brief  頻繁に使用されるシェーダーを読み込むための便利な関数。内部で ShaderLibrary を使用する。
		/// @param  key シェーダーのキー（ShaderLibrary に登録されている必要がある）
		/// @param  csoPath CSOファイルのパス（ワイド文字列）。ベースディレクトリからの相対パスも可。
		/// @return 読み込まれたシェーダー（失敗した場合は nullptr）
        std::shared_ptr<VertexShader> LoadVertexShader(const std::wstring& key, const std::wstring& csoPath);
		/// @brief  頻繁に使用されるシェーダーを読み込むための便利な関数。内部で ShaderLibrary を使用する。
		/// @param  key シェーダーのキー（ShaderLibrary に登録されている必要がある）
		/// param  csoPath CSOファイルのパス（ワイド文字列）。ベースディレクトリからの相対パスも可。
		/// @return 読み込まれたシェーダー（失敗した場合は nullptr）
        std::shared_ptr<PixelShader> LoadPixelShader(const std::wstring& key, const std::wstring& csoPath);

		/// @brief  入力レイアウトを生成するための便利な関数。内部で ShaderLibrary を使用する。
		/// @param  layoutKey 入力レイアウトのキー（ShaderLibrary に登録されている必要がある）
		/// @param  layout 入力レイアウト定義
		/// @param  vertexShader 入力レイアウトを作成するための頂点シェーダー
		/// @return 生成された入力レイアウト（失敗した場合は nullptr）
        std::shared_ptr<VertexShader> LoadVertexShader(const std::wstring& csoPath);
		/// @brief  頻繁に使用されるシェーダーを読み込むための便利な関数。内部で ShaderLibrary を使用する。
		/// @param  csoPath CSOファイルのパス（ワイド文字列）。ベースディレクトリからの相対パスも可。
		/// @return 読み込まれたシェーダー（失敗した場合は nullptr）
        std::shared_ptr<PixelShader> LoadPixelShader(const std::wstring& csoPath);

		/// @brief  入力レイアウトを生成するための便利な関数。内部で ShaderLibrary を使用する。
		/// @param  layoutKey 入力レイアウトのキー（ShaderLibrary に登録されている必要がある）
		/// @param  layout 入力レイアウト定義
		/// @param  vertexShader 入力レイアウトを作成するための頂点シェーダー
		/// @return 生成された入力レイアウト（失敗した場合は nullptr）
        std::shared_ptr<InputLayout> CreateInputLayout(
            const std::wstring& layoutKey,
            const VertexInputLayout& layout,
            const VertexShader& vertexShader
        );

		/// @brief  シェーダーライブラリへのポインタを取得する。
		/// @return シェーダーライブラリへのポインタ
        ShaderLibrary* GetShaderLibrary();
		/// @brief  シェーダーライブラリへのポインタを取得する（const）。
		/// @return シェーダーライブラリへのポインタ
        const ShaderLibrary* GetShaderLibrary() const;

        //============================================================
        // Model (OBJ/FBX/GLTF/DAE supported via Assimp)
        //============================================================
		/// @brief  モデルを読み込む。すでに同じパスで読み込まれているモデルがあれば、それを再利用する。
		/// @param  path モデルファイルのパス（ワイド文字列）。ベースディレクトリからの相対パスも可。
		/// @return 読み込まれたモデル（失敗した場合は nullptr）
        std::shared_ptr<Model> LoadModel(const std::wstring& path);
		/// @brief  使用されていないモデルをキャッシュから削除する。共有ポインタが切れている（参照カウントが0の）モデルが対象。
        void PruneUnusedModels();

		/// @brief  D3D11デバイスへのポインタを取得
		/// @return D3D11デバイスへのポインタ
        ID3D11Device* GetDevice() const;

    private:
		/// @brief  パスを解決する。ベースディレクトリが設定されている場合は、ベースディレクトリからの相対パスを絶対パスに変換する。
		/// @param  path ファイルパス（ワイド文字列）。ベースディレクトリからの相対パスも可。
		/// @return 解決されたファイルパス（ワイド文字列）
        std::wstring ResolvePath(const std::wstring& path) const;
		/// @brief  テクスチャキャッシュのキーを生成する。解決されたパスとオプションを組み合わせて一意のキーを作成する。
        /// @param  resolvedPath 解決されたファイルパス（ワイド文字列）
        /// @param  options テクスチャ読み込みオプション
		/// @return テクスチャキャッシュのキー（ワイド文字列）
        std::wstring MakeTextureCacheKey(const std::wstring& resolvedPath, const TextureLoadOptions& options) const;
		/// @brief  モデルキャッシュのキーを生成する。解決されたパスをそのままキーとして使用する。
		/// @param  resolvedPath 解決されたファイルパス（ワイド文字列）
		/// @return モデルキャッシュのキー（ワイド文字列）
        std::wstring MakeModelCacheKey(const std::wstring& resolvedPath) const;

    private:
        bool m_isInitialized            = false;    /// 初期化済みフラグ

        ID3D11Device* m_device          = nullptr;  /// D3D11デバイスは外部管理

        std::wstring m_baseDirectory    = L"";      /// ベースディレクトリ（相対パスの基準）

		ShaderLibrary m_shaderLibrary;              /// シェーダーライブラリ

		std::unordered_map<std::wstring, std::weak_ptr<Texture>> m_textureCache;    /// テクスチャキャッシュ（weak_ptrで管理し、PruneUnusedTextures()で切れたものを削除）
		std::unordered_map<std::wstring, std::weak_ptr<Model>>   m_modelCache;      /// モデルキャッシュ（weak_ptrで管理し、PruneUnusedModels()で切れたものを削除）

		mutable std::mutex m_mutex;                 /// キャッシュアクセス用のミューテックス（テクスチャとモデル両方を保護）
    };

} // namespace Engine
