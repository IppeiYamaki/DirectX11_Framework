#pragma once

#include <d3d11.h>
#include <memory>

#include "Engine/Scene/Component.h"
#include "Engine/Graphics/Mesh.h"
#include "Engine/Graphics/MeshFactory.h"
#include "Engine/Graphics/RenderLayer.h"

namespace Engine {

    class RenderSystem;
    class Material;

    /// @brief  MeshとMaterialを保持し、RenderSystemへ描画要求を出す
    class MeshRenderer final : public Component {
    public:
        /// @brief  コンストラクタ
        /// @param  device D3D11デバイス
        /// @param  renderSystem RenderSystem参照
        MeshRenderer(ID3D11Device* device, RenderSystem* renderSystem);
        ~MeshRenderer() override = default;

        /// @brief Start時に呼ばれる
        void OnStart() override;
        /// @brief 描画時に呼ばれる
        void Draw() override;
        /// @brief 破棄時に呼ばれる
        void OnDestroy() override;

        /// @brief  メッシュタイプを設定
        /// @param  type メッシュタイプ
        void SetMeshType(MeshType type);
        /// @brief  メッシュ生成情報を設定
        /// @param  desc メッシュ生成情報
        void SetMeshDesc(const MeshCreateDesc& desc);

        /// @brief  マテリアルを設定
        /// @param  material マテリアル
        void SetMaterial(const std::shared_ptr<Material>& material);
        /// @brief  マテリアルを取得
        /// @return マテリアル（借用）
        Material* GetMaterial() const;

        /// @brief  描画レイヤーを設定
        /// @param  layer 描画レイヤー
        void SetRenderLayer(RenderLayer layer);
        /// @brief  描画レイヤーを取得
        /// @return 描画レイヤー
        RenderLayer GetRenderLayer() const;

        /// @brief  レイヤー内描画順を設定
        /// @param  orderInLayer レイヤー内描画順
        void SetOrderInLayer(int orderInLayer);
        /// @brief  レイヤー内描画順を取得
        /// @return レイヤー内描画順
        int GetOrderInLayer() const;

        /// @brief  描画ステートフラグを設定
        /// @param  flags 描画ステートフラグ
        void SetRenderStateFlags(std::uint32_t flags);
        /// @brief  描画ステートフラグを取得
        /// @return 描画ステートフラグ
        std::uint32_t GetRenderStateFlags() const;

    private:
        /// @brief メッシュ生成が必要なら生成
        void CreateMeshIfNeeded();

    private:
        ID3D11Device* m_device = nullptr;       ///< D3D11デバイス（借用）
        RenderSystem* m_renderSystem = nullptr; ///< RenderSystem（借用）

        std::shared_ptr<Material> m_material;   ///< マテリアル

        MeshCreateDesc m_meshDesc{};            ///< メッシュ生成情報
        bool m_isMeshDirty = true;              ///< 生成情報の更新フラグ
        bool m_isMeshReady = false;             ///< 生成済みフラグ

        Mesh m_mesh;                            ///< メッシュ本体

        RenderLayer  m_renderLayer = RenderLayer::Opaque; ///< 描画レイヤー
        int          m_orderInLayer = 0;                  ///< レイヤー内描画順
        std::uint32_t m_stateFlags = kRenderStateNone;    ///< 描画ステートフラグ
    };

} // namespace Engine
