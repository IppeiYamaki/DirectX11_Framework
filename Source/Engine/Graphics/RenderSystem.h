#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>

#include <vector>
#include <cstdint>

#include "Engine/Graphics/ConstantBuffer.h"
#include "Engine/Graphics/ShaderConstants.h"
#include "Engine/Graphics/RenderLayer.h"

namespace Engine {

    class GraphicsDevice;
    class World;
    class Mesh;
    class Material;

    /// @brief  描画要求（RenderQueueの1要素）
    struct RenderItem final {
        /// @brief 描画するメッシュ（借用）
        Mesh* m_mesh = nullptr;
        /// @brief 描画に使用するマテリアル（借用）
        Material* m_material = nullptr;
        /// @brief ワールド行列
        DirectX::XMFLOAT4X4         m_world{};
        /// @brief プリミティブトポロジー
        D3D11_PRIMITIVE_TOPOLOGY    m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        /// @brief 描画レイヤー（Unity風）
        RenderLayer                 m_layer = RenderLayer::Opaque;
        /// @brief レイヤー内の描画順
        int                         m_orderInLayer = 0;

        /// @brief 描画ステートフラグ
        std::uint32_t               m_stateFlags = kRenderStateNone;

        /// @brief デフォルトコンストラクタ（行列を単位化）
        RenderItem() {
            DirectX::XMStoreFloat4x4(&m_world, DirectX::XMMatrixIdentity());
        }
    };

    /// @brief  描画キューを管理し、1フレームの描画を統括する
    class RenderSystem final {
    public:
        RenderSystem() = default;
        ~RenderSystem() = default;

        RenderSystem(const RenderSystem&) = delete;
        RenderSystem& operator=(const RenderSystem&) = delete;

        /// @brief  初期化
        /// @param  graphicsDevice GraphicsDevice参照
        /// @return 成功ならtrue
        bool Initialize(GraphicsDevice& graphicsDevice);
        /// @brief 終了処理
        void Finalize();
        /// @brief リセット
        void Reset();

        /// @brief  描画実行
        /// @param  world 描画対象World
        void Draw(World& world);

        /// @brief  初期化済みか確認
        /// @return 初期化済みならtrue
        bool IsInitialized() const;

        //============================================================
        // RenderQueue
        //============================================================
        /// @brief  描画要求を追加
        /// @param  item 追加する描画要求
        void AddRenderItem(const RenderItem& item);
        /// @brief 描画要求をクリア
        void ClearRenderItems();

        //============================================================
        // Frame constants (View/Projection/Light)
        //============================================================
        /// @brief  ビュー行列を設定
        /// @param  view ビュー行列
        void SetViewMatrix(const DirectX::XMFLOAT4X4& view);
        /// @brief  射影行列を設定
        /// @param  projection 射影行列
        void SetProjectionMatrix(const DirectX::XMFLOAT4X4& projection);
        /// @brief  ディレクショナルライトを設定
        /// @param  light ライト情報
        void SetLight(const DirectionalLight& light);

    private:
        /// @brief  フレーム定数バッファをバインド
        /// @param  context デバイスコンテキスト
        void BindFrameConstants(ID3D11DeviceContext* context); // b1,b2,b4
        /// @brief  描画ステートを適用
        /// @param  context デバイスコンテキスト
        /// @param  item 描画要求
        void ApplyRenderStates(ID3D11DeviceContext* context, const RenderItem& item);
        /// @brief  描画要求を実行
        /// @param  context デバイスコンテキスト
        /// @param  item 描画要求
        void DrawItem(ID3D11DeviceContext* context, const RenderItem& item);

    private:
        GraphicsDevice* m_graphicsDevice = nullptr; ///< GraphicsDevice（借用）
        bool m_isInitialized = false;              ///< 初期化済みフラグ

        std::vector<RenderItem> m_items;           ///< 描画キュー

        ConstantBuffer<WorldCB>         m_worldCb; ///< b0
        ConstantBuffer<ViewCB>          m_viewCb;  ///< b1
        ConstantBuffer<ProjectionCB>    m_projCb;  ///< b2
        ConstantBuffer<LightCB>         m_lightCb; ///< b4

        ViewCB       m_viewData{};       ///< ビュー定数
        ProjectionCB m_projData{};       ///< 射影定数
        LightCB      m_lightData{};      ///< ライト定数

        /// @brief 深度ステンシルステート群
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_dsDefault;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_dsDepthWriteOff;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_dsDepthOff;

        /// @brief ラスタライザーステート群
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rsCullBack;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rsCullFront;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rsCullNone;

        /// @brief ブレンドステート群
        Microsoft::WRL::ComPtr<ID3D11BlendState> m_bsOpaque;
        Microsoft::WRL::ComPtr<ID3D11BlendState> m_bsAlpha;

    private:
        static constexpr float kDefaultClearColor[4] = { 0.10f, 0.10f, 0.18f, 1.0f };
    };

} // namespace Engine
