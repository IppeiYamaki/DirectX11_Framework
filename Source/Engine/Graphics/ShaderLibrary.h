#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "Engine/Graphics/Shader.h"

namespace Engine {

	/// @brief シェーダーと入力レイアウトのキャッシュ管理を行うクラス
	/// @brief LoadVertexShader / LoadPixelShader はキャッシュして同一キーは同一インスタンスを返す
    class ShaderLibrary final {
    public:
        ShaderLibrary() = default;
        ~ShaderLibrary();

        ShaderLibrary(const ShaderLibrary&) = delete;
        ShaderLibrary& operator=(const ShaderLibrary&) = delete;

		/// @brief  初期化
		/// @param  device D3D11デバイス
		/// @return 成功ならtrue
        bool Initialize(ID3D11Device* device);
		/// @brief  終了処理
        void Finalize();
		/// @brief  リセット
        void Reset();

		/// @brief  頂点シェーダーを読み込み、キャッシュに保存
		/// @param  key キャッシュキー
        /// @param  csoPath CSOファイルパス
        /// @return 読み込んだシェーダー
        std::shared_ptr<VertexShader> LoadVertexShader  (const std::wstring& key, const std::wstring& csoPath);
		/// @brief  ピクセルシェーダーを読み込み、キャッシュに保存
		/// @param  key キャッシュキー
		/// @param  csoPath CSOファイルパス
        /// @return 読み込んだシェーダー
        std::shared_ptr<PixelShader> LoadPixelShader    (const std::wstring& key, const std::wstring& csoPath);

		/// @brief  入力レイアウトを生成し、キャッシュに保存
		/// @param  layoutKey キャッシュキー
        /// @param  layout 入力レイアウト情報
        /// @param  vertexShader 頂点シェーダー
        /// @return 生成した入力レイアウト
        std::shared_ptr<InputLayout> CreateInputLayout(
            const std::wstring& layoutKey,
            const VertexInputLayout& layout,
            const VertexShader& vertexShader
        );

        /// @brief  D3D11デバイスを取得
		/// @return D3D11デバイス
        ID3D11Device* GetDevice() const;

    private:
		ID3D11Device* m_device = nullptr;   /// デバイスは外部管理

		std::unordered_map<std::wstring, std::shared_ptr<VertexShader>> m_vsCache;      /// 頂点シェーダーキャッシュ
        std::unordered_map<std::wstring, std::shared_ptr<PixelShader>>  m_psCache;      /// ピクセルシェーダーキャッシュ
        std::unordered_map<std::wstring, std::shared_ptr<InputLayout>>  m_layoutCache;  /// 入力レイアウトキャッシュ

		bool m_isInitialized = false;       /// 初期化済みフラグ
    };

} // namespace Engine
