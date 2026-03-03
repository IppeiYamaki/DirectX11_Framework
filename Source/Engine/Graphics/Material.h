#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <array>
#include <memory>

#include "Engine/Graphics/ConstantBuffer.h"
#include "Engine/Graphics/ShaderConstants.h"
#include "Engine/Graphics/Shader.h"
#include "Engine/Graphics/Texture.h"

namespace Engine {

    /// @brief  描画に必要な状態をまとめるマテリアル
    /// @note   Shader/Texture/SamplerとMaterial定数(b3)を保持する
    /// @note   派生クラス（TerrainBlendMaterialなど）で多態的なBind処理を実現するため、virtualを使用
    class Material {
    public:
        Material() = default;
        virtual ~Material();

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
        // TerrainBlend params (b7) - optional
        //============================================================
        /// @brief  地形ブレンド定数バッファを有効化・初期化
        /// @param  device D3D11デバイス
        /// @return 成功ならtrue
        bool EnableTerrainBlendBuffer(ID3D11Device* device);

        /// @brief  地形ブレンドパラメータを取得
        /// @return 地形ブレンドパラメータへの参照（未初期化の場合はデフォルト）
        TerrainBlendParams& GetTerrainBlendParams();
        
        /// @brief  地形ブレンドパラメータを取得（const版）
        /// @return 地形ブレンドパラメータへの参照
        const TerrainBlendParams& GetTerrainBlendParams() const;

        /// @brief  地形ブレンドバッファが有効か確認
        /// @return 有効ならtrue
        bool IsTerrainBlendEnabled() const { return m_useTerrainBlend; }

        //============================================================
        // Terrain Layer Textures (t0-t3 for TerrainBlend)
        //============================================================
        /// @brief  地形レイヤーテクスチャを設定
        /// @param  layerIndex レイヤーインデックス（0～3）
        /// @param  texture テクスチャ
        void SetTerrainLayerTexture(std::uint32_t layerIndex, const std::shared_ptr<Texture>& texture);

        /// @brief  地形レイヤーテクスチャを取得
        /// @param  layerIndex レイヤーインデックス（0～3）
        /// @return テクスチャ（未設定ならnullptr）
        std::shared_ptr<Texture> GetTerrainLayerTexture(std::uint32_t layerIndex) const;

        //============================================================
        // Bind
        //============================================================
        /// @brief  パイプラインへバインド
        /// @param  context デバイスコンテキスト
        /// @note   派生クラスでoverrideして追加のバインド処理を実装可能
        virtual void Bind(ID3D11DeviceContext* context);


    protected:
        /// @brief  既定サンプラーを生成
        /// @param  device D3D11デバイス
        /// @return 成功ならtrue
        bool CreateDefaultSampler(ID3D11Device* device);

    protected:
        bool m_isInitialized = false;                   /// 初期化済みフラグ
        bool m_firstBindLogged = false;                 /// 最初のBindログ出力済みフラグ

        std::shared_ptr<VertexShader> m_vs;             /// 頂点シェーダー
        std::shared_ptr<PixelShader>  m_ps;             /// ピクセルシェーダー
        std::shared_ptr<InputLayout>  m_inputLayout;    /// 入力レイアウト

        std::shared_ptr<Texture> m_texture;             /// テクスチャ

        /// @brief 外部サンプラー（nullptrならデフォルト）
        ID3D11SamplerState* m_samplerExternal = nullptr;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_defaultSampler; /// デフォルトサンプラー

        ConstantBuffer<MaterialCB> m_materialCb; /// b3 定数バッファ
        MaterialCB m_materialData{};             /// マテリアル定数

        // TerrainBlend定数バッファ（オプション）
        bool m_useTerrainBlend = false;                         /// 地形ブレンド使用フラグ
        std::unique_ptr<ConstantBuffer<TerrainBlendCB>> m_terrainBlendCb; /// b7 定数バッファ
        TerrainBlendCB m_terrainBlendData{};                    /// 地形ブレンド定数

        // 地形レイヤーテクスチャ（t0～t3）
        std::array<std::shared_ptr<Texture>, kMaxTerrainLayers> m_terrainLayerTextures;
    };

} // namespace Engine
