#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <memory>

#include "Engine/Graphics/ConstantBuffer.h"
#include "Engine/Graphics/ShaderConstants.h"
#include "Engine/Graphics/Shader.h"
#include "Engine/Graphics/Texture.h"

namespace Engine {

    /// @brief  描画に必要な状態をまとめるマテリアル
    /// @note   Shader/Texture/SamplerとMaterial定数(b3)を保持する
    class Material final {
    public:
        Material() = default;
        ~Material();

        Material(const Material&) = delete;
        Material& operator=(const Material&) = delete;

        //============================================================
        // Lifecycle
        //============================================================
        /// @brief  初期化
        /// @param  device D3D11デバイス
        /// @return 成功ならtrue
        bool Initialize(ID3D11Device* device);
        /// @brief 終了処理
        void Finalize();

        /// @brief  初期化済みか確認
        /// @return 初期化済みならtrue
        bool IsInitialized() const;

        //============================================================
        // Set pipeline resources
        //============================================================
        /// @brief  頂点シェーダーを設定
        /// @param  vs 頂点シェーダー
        void SetVertexShader(const std::shared_ptr<VertexShader>& vs);
        /// @brief  ピクセルシェーダーを設定
        /// @param  ps ピクセルシェーダー
        void SetPixelShader(const std::shared_ptr<PixelShader>& ps);
        /// @brief  入力レイアウトを設定
        /// @param  inputLayout 入力レイアウト
        void SetInputLayout(const std::shared_ptr<InputLayout>& inputLayout);

        //============================================================
        // Texture/Sampler
        //============================================================
        /// @brief  テクスチャを設定
        /// @param  texture テクスチャ
        void SetTexture(const std::shared_ptr<Texture>& texture);
        /// @brief  サンプラーを設定
        /// @param  sampler サンプラー（nullptrでデフォルト）
        void SetSampler(ID3D11SamplerState* sampler);

        //============================================================
        // Material params (b3)
        //============================================================
        /// @brief  マテリアルパラメータを取得
        /// @return マテリアルパラメータへの参照
        MaterialParams& GetParams();
        /// @brief  マテリアルパラメータを取得（const版）
        /// @return マテリアルパラメータへの参照
        const MaterialParams& GetParams() const;

        /// @brief  テクスチャ使用を切り替える
        /// @param  isEnabled 有効フラグ
        void EnableTexture(bool isEnabled);
        /// @brief  アルファテストを設定
        /// @param  isEnabled 有効フラグ
        /// @param  alphaCutoff 破棄閾値
        void EnableAlphaTest(bool isEnabled, float alphaCutoff = 0.5f);

        //============================================================
        // Bind
        //============================================================
        /// @brief  パイプラインへバインド
        /// @param  context デバイスコンテキスト
        void Bind(ID3D11DeviceContext* context);


    private:
        /// @brief  既定サンプラーを生成
        /// @param  device D3D11デバイス
        /// @return 成功ならtrue
        bool CreateDefaultSampler(ID3D11Device* device);

    private:
        bool m_isInitialized = false; ///< 初期化済みフラグ

        std::shared_ptr<VertexShader> m_vs;       ///< 頂点シェーダー
        std::shared_ptr<PixelShader>  m_ps;       ///< ピクセルシェーダー
        std::shared_ptr<InputLayout>  m_inputLayout; ///< 入力レイアウト

        std::shared_ptr<Texture> m_texture; ///< テクスチャ

        /// @brief 外部サンプラー（nullptrならデフォルト）
        ID3D11SamplerState* m_samplerExternal = nullptr;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_defaultSampler; ///< デフォルトサンプラー

        ConstantBuffer<MaterialCB> m_materialCb; ///< b3 定数バッファ
        MaterialCB m_materialData{};             ///< マテリアル定数
    };

} // namespace Engine
