/// @file   TerrainBlendMaterial.h
/// @brief  地形用マルチテクスチャブレンドマテリアル
/// @note   複数のテクスチャ（草、土、砂、岩など）をブレンドして地形を描画する
#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <array>
#include <string>

#include "Engine/Graphics/Material.h"
#include "Engine/Graphics/ConstantBuffer.h"
#include "Engine/Graphics/ShaderConstants.h"
#include "Engine/Materials/MaterialBuildContext.h"

namespace Engine {
    class Texture;
    class VertexShader;
    class PixelShader;
    class InputLayout;
}

namespace Engine {

    /// @brief 地形用マルチテクスチャブレンドマテリアル
    /// @note  Materialを継承し、Bindをoverrideして地形ブレンド専用の処理を行う
    /// @note  最大4レイヤーのテクスチャをブレンドウェイトで混合する
    /// 
    /// 使用例（Prefabでの設定例）：
    /// @code
    /// // TerrainBlendMaterialをMaterialLibrary経由で取得
    /// auto terrainMat = ctx.m_materials->GetOrCreate<TerrainBlendMaterialAsset>();
    /// auto* blendMat = dynamic_cast<TerrainBlendMaterial*>(terrainMat.get());
    /// 
    /// // レイヤー0に草テクスチャを設定
    /// blendMat->SetLayerTexture(0, grassTexture);
    /// // レイヤー1に土テクスチャを設定
    /// blendMat->SetLayerTexture(1, dirtTexture);
    /// 
    /// // ブレンドウェイトを設定（草50%, 土30%, 砂20%）
    /// blendMat->SetBlendWeights(0.5f, 0.3f, 0.2f, 0.0f);
    /// 
    /// // 各レイヤーのUVスケール（タイリング倍率）を設定
    /// blendMat->SetLayerUVScale(0, 20.0f);  // 草は細かく
    /// blendMat->SetLayerUVScale(1, 15.0f);  // 土
    /// @endcode
    class TerrainBlendMaterial : public Material {
    public:
        /// @brief MaterialLibrary用識別子
        static constexpr const char* kName = "TerrainBlendMaterial";

        TerrainBlendMaterial() = default;
        ~TerrainBlendMaterial() override;

        TerrainBlendMaterial(const TerrainBlendMaterial&) = delete;
        TerrainBlendMaterial& operator=(const TerrainBlendMaterial&) = delete;

        //============================================================
        // Lifecycle
        //============================================================

        /// @brief 初期化
        /// @param device D3D11デバイス
        /// @return 成功ならtrue
        bool Initialize(ID3D11Device* device);

        /// @brief 終了処理
        void Finalize();

        //============================================================
        // Texture Layer Management
        //============================================================

        /// @brief レイヤーにテクスチャを設定
        /// @param layerIndex レイヤーインデックス（0～3）
        /// @param texture テクスチャ
        void SetLayerTexture(std::uint32_t layerIndex, const std::shared_ptr<Texture>& texture);

        /// @brief レイヤーのテクスチャを取得
        /// @param layerIndex レイヤーインデックス（0～3）
        /// @return テクスチャ（未設定ならnullptr）
        std::shared_ptr<Texture> GetLayerTexture(std::uint32_t layerIndex) const;

        //============================================================
        // Blend Weight Management
        //============================================================

        /// @brief ブレンドウェイトを一括設定
        /// @param w0 レイヤー0のウェイト
        /// @param w1 レイヤー1のウェイト
        /// @param w2 レイヤー2のウェイト
        /// @param w3 レイヤー3のウェイト
        /// @note  合計が1.0になるように正規化されます
        void SetBlendWeights(float w0, float w1, float w2, float w3);

        /// @brief 個別レイヤーのブレンドウェイトを設定
        /// @param layerIndex レイヤーインデックス（0～3）
        /// @param weight ウェイト値
        void SetLayerBlendWeight(std::uint32_t layerIndex, float weight);

        /// @brief ブレンドウェイトを取得
        /// @return ブレンドウェイト（x,y,z,w = Layer 0,1,2,3）
        const Vector4& GetBlendWeights() const;

        //============================================================
        // UV Scale Management
        //============================================================

        /// @brief 全レイヤーのUVスケールを一括設定
        /// @param scale0 レイヤー0のUVスケール
        /// @param scale1 レイヤー1のUVスケール
        /// @param scale2 レイヤー2のUVスケール
        /// @param scale3 レイヤー3のUVスケール
        void SetUVScales(float scale0, float scale1, float scale2, float scale3);

        /// @brief 個別レイヤーのUVスケールを設定
        /// @param layerIndex レイヤーインデックス（0～3）
        /// @param scale UVスケール（タイリング倍率）
        void SetLayerUVScale(std::uint32_t layerIndex, float scale);

        /// @brief UVスケールを取得
        /// @return UVスケール（x,y,z,w = Layer 0,1,2,3）
        const Vector4& GetUVScales() const;

        //============================================================
        // Layer Tint Management
        //============================================================

        /// @brief レイヤーの色調を設定
        /// @param layerIndex レイヤーインデックス（0～3）
        /// @param tint 色調（RGBA）
        void SetLayerTint(std::uint32_t layerIndex, const Vector4& tint);

        /// @brief レイヤーの色調を取得
        /// @param layerIndex レイヤーインデックス（0～3）
        /// @return 色調（RGBA）
        const Vector4& GetLayerTint(std::uint32_t layerIndex) const;

        //============================================================
        // Active Layer Count
        //============================================================

        /// @brief 有効なレイヤー数を設定
        /// @param count レイヤー数（1～4）
        void SetActiveLayerCount(std::uint32_t count);

        /// @brief 有効なレイヤー数を取得
        /// @return レイヤー数
        std::uint32_t GetActiveLayerCount() const;

        //============================================================
        // Binding (override from Material)
        //============================================================

        /// @brief パイプラインへバインド（多態的処理）
        /// @param context デバイスコンテキスト
        /// @note  Materialの基底クラスからvirtualで呼び出される
        void Bind(ID3D11DeviceContext* context) override;

        //============================================================
        // MaterialLibrary Factory Support
        //============================================================

        /// @brief MaterialLibrary用のファクトリ関数
        /// @param ctx マテリアルビルドコンテキスト
        /// @return 生成されたマテリアル（TerrainBlendMaterial型）
        static std::shared_ptr<Material> Create(const MaterialBuildContext& ctx);

    private:
        /// @brief 定数バッファを更新
        void UpdateConstantBuffer(ID3D11DeviceContext* context);

    private:
        // TerrainBlend専用パラメータ
        TerrainBlendParams m_terrainParams{};

        // Dirty flag for constant buffer update
        bool m_isDirty = true;
        
        // 最初のBindログ出力済みフラグ
        bool m_terrainFirstBindLogged = false;
    };

    //=========================================================================
    // TerrainBlendMaterialAsset
    // MaterialLibrary経由で使用するためのアセットクラス
    //=========================================================================
    
    /// @brief MaterialLibraryで管理するためのTerrainBlendMaterialアセット定義
    /// @note  MaterialLibrary::GetOrCreate<TerrainBlendMaterialAsset>() で使用
    class TerrainBlendMaterialAsset final {
    public:
        static constexpr const char* kName = "TerrainBlendMaterial";

        /// @brief TerrainBlendMaterialを生成して返す
        static std::shared_ptr<Material> Create(const MaterialBuildContext& ctx);
    };

} // namespace Engine
