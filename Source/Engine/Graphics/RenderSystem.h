#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>

#include <vector>
#include <cstdint>
#include <memory>
#include <functional>

#include "Engine/Graphics/ConstantBuffer.h"
#include "Engine/Graphics/ShaderConstants.h"
#include "Engine/Graphics/RenderLayer.h"
#include "Engine/Graphics/Shader.h"
#include "Engine/Graphics/Mesh.h"

namespace Engine {

    class GraphicsDevice;
    class World;
    class Material;
    class Texture;
    class Canvas;
    class RenderTarget;
    class FadeSystem;
    class DebugVisualizationSystem;
    class DebugImGuiSystem;
    class Scene;
    class LightSystem;


    /// @brief  描画要求（RenderQueueの1要素）
    struct RenderItem final {
        /// @brief 描画するメッシュ
        Mesh* m_mesh = nullptr;
        /// @brief 描画に使用するマテリアル
        Material* m_material = nullptr;
        /// @brief ワールド行列
        DirectX::XMFLOAT4X4         m_world{};
        /// @brief プリミティブトポロジー
        D3D11_PRIMITIVE_TOPOLOGY    m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        /// @brief 描画レイヤー
		RenderLayer                 m_layer = RenderLayer::Opaque;
        /// @brief レイヤー内の描画順
        int                         m_orderInLayer = 0;

        /// @brief 描画ステートフラグ
        std::uint32_t               m_stateFlags = kRenderStateNone;

        /// @brief デフォルトコンストラクタ（行列を単位化）
        RenderItem() {
            DirectX::XMStoreFloat4x4(&m_world, DirectX::XMMatrixIdentity());
        }

        /// @brief 反射パスから除外するかどうか
        bool m_excludeFromReflection = false;
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
        /// @param  canvas UI描画用Canvas（nullptrの場合はUI描画をスキップ）
        /// @param  fadeSystem フェード描画用FadeSystem（nullptrの場合はフェード描画をスキップ）
        /// @param  debugVisualization デバッグ可視化システム（nullptrの場合はデバッグ描画をスキップ）
        /// @param  scene デバッグ描画用Scene（nullptrの場合はシーン関連のデバッグ描画をスキップ）
        /// @param  lightSystem デバッグ描画用LightSystem（nullptrの場合はライト関連のデバッグ描画をスキップ）
        /// @param  debugImGui デバッグImGuiシステム（nullptrの場合はImGui描画をスキップ）
        void Draw(World& world, Canvas* canvas = nullptr, FadeSystem* fadeSystem = nullptr,
                  DebugVisualizationSystem* debugVisualization = nullptr, Scene* scene = nullptr, 
                  LightSystem* lightSystem = nullptr, DebugImGuiSystem* debugImGui = nullptr);

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
        /// @brief  ディレクショナルライトを設定（レガシーAPI）
        /// @param  light ライト情報
        void SetLight(const DirectionalLight& light);

        /// @brief  拡張ライトデータを設定（複数ライト対応）
        /// @param  extendedLightData 拡張ライトデータ
        void SetExtendedLights(const ExtendedLightCB& extendedLightData);

        /// @brief  拡張ライトデータを取得
        /// @return 拡張ライトデータへの参照
        [[nodiscard]] const ExtendedLightCB& GetExtendedLights() const { return m_extendedLightData; }

        /// @brief  拡張ライトモードを有効化/無効化
        /// @param  enable 有効にする場合true
        void SetExtendedLightingEnabled(bool enable);

        /// @brief  拡張ライトモードが有効か確認
        /// @return 有効ならtrue
        [[nodiscard]] bool IsExtendedLightingEnabled() const { return m_extendedLightingEnabled; }

        //============================================================
        // UI/2D Sprite Rendering
        //============================================================
        /// @brief  2Dスプライト（UI画像）を描画
        /// @param  texture 描画するテクスチャ
        /// @param  left 左端座標（スクリーン座標）
        /// @param  top 上端座標（スクリーン座標）
        /// @param  right 右端座標（スクリーン座標）
        /// @param  bottom 下端座標（スクリーン座標）
        /// @param  color 乗算カラー（RGBA）
        void DrawSprite(Texture* texture, float left, float top, float right, float bottom,
                        float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);

        /// @brief  単色の矩形を描画（フェードオーバーレイ等に使用）
        /// @param  left 左端座標（スクリーン座標）
        /// @param  top 上端座標（スクリーン座標）
        /// @param  right 右端座標（スクリーン座標）
        /// @param  bottom 下端座標（スクリーン座標）
        /// @param  r 赤成分（0.0～1.0）
        /// @param  g 緑成分（0.0～1.0）
        /// @param  b 青成分（0.0～1.0）
        /// @param  a アルファ成分（0.0～1.0）
        void DrawColoredRect(float left, float top, float right, float bottom,
                             float r, float g, float b, float a);

        /// @brief  GraphicsDeviceを取得
        /// @return GraphicsDevice
        GraphicsDevice* GetGraphicsDevice() const { return m_graphicsDevice; }

        //============================================================
        // Render Target Management (for reflection etc.)
        //============================================================

        /// @brief  指定のレンダーターゲットへの描画を開始
        /// @param  target レンダーターゲット
        /// @param  clearColor クリアカラー（nullptrの場合はクリアしない）
        void BeginRenderTarget(RenderTarget* target, const float* clearColor = nullptr);

        /// @brief  レンダーターゲットへの描画を終了（バックバッファに戻す）
        void EndRenderTarget();

        /// @brief  現在のアイテムキューをソートして描画（RTに描画可能）
        /// @param  excludeReflectionItems 反射除外アイテムを除外するか
        void FlushRenderItems(bool excludeReflectionItems = false);

        /// @brief  ビュー行列を取得
        /// @return ビュー行列
        const DirectX::XMFLOAT4X4& GetViewMatrix() const { return m_viewData.g_view; }

        /// @brief  射影行列を取得
        /// @return 射影行列
        const DirectX::XMFLOAT4X4& GetProjectionMatrix() const { return m_projData.g_projection; }

        /// @brief  クリップ平面を設定（反射パス用）
        /// @param  plane クリップ平面（x,y,z=法線、w=距離）
        void SetClipPlane(const DirectX::XMFLOAT4& plane);

        /// @brief  クリップ平面を無効化
        void ClearClipPlane();

        /// @brief  クリップ平面が有効かどうか
        /// @return 有効ならtrue
        bool IsClipPlaneEnabled() const { return m_clipPlaneEnabled; }

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

        /// @brief  UI描画用リソースを初期化
        /// @return 成功ならtrue
        bool InitializeUIResources();
        /// @brief  UI描画用リソースを解放
        void FinalizeUIResources();

    private:
        GraphicsDevice* m_graphicsDevice = nullptr;     /// GraphicsDevice
        bool m_isInitialized = false;                   /// 初期化済みフラグ

        std::vector<RenderItem> m_items;                /// 描画キュー

        ConstantBuffer<WorldCB>         m_worldCb;      /// b0
        ConstantBuffer<ViewCB>          m_viewCb;       /// b1
        ConstantBuffer<ProjectionCB>    m_projCb;       /// b2
        ConstantBuffer<LightCB>         m_lightCb;      /// b4
        ConstantBuffer<ExtendedLightCB> m_extendedLightCb;  /// b7 拡張ライト

        ViewCB                          m_viewData{};   /// ビュー定数
        ProjectionCB                    m_projData{};   /// 射影定数
        LightCB                         m_lightData{};  /// ライト定数
        ExtendedLightCB                 m_extendedLightData{};  /// 拡張ライト定数
        bool                            m_extendedLightingEnabled = false;  /// 拡張ライティング有効フラグ

        /// @brief 深度ステンシルステート群
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_dsDefault;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_dsDepthWriteOff;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_dsDepthOff;

        /// @brief ラスタライザーステート群
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rsCullBack;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rsCullFront;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rsCullNone;

        /// @brief ブレンドステート群
        Microsoft::WRL::ComPtr<ID3D11BlendState>      m_bsOpaque;
        Microsoft::WRL::ComPtr<ID3D11BlendState>      m_bsAlpha;
        Microsoft::WRL::ComPtr<ID3D11BlendState>      m_bsAdditive;

        //============================================================
        // UI Rendering Resources
        //============================================================
        std::unique_ptr<VertexShader> m_uiVertexShader;     /// UI用頂点シェーダー
        std::unique_ptr<PixelShader> m_uiPixelShader;       /// UI用ピクセルシェーダー
        std::unique_ptr<InputLayout> m_uiInputLayout;       /// UI用入力レイアウト
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_uiVertexBuffer;    /// UI用動的頂点バッファ
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_uiConstantBuffer;  /// UI用定数バッファ
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_uiSampler;   /// UI用サンプラーステート
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_whiteDummySRV;  /// 1x1白テクスチャSRV（単色描画用）
        bool m_uiResourcesInitialized = false;              /// UI描画リソース初期化済みフラグ

        //============================================================
        // Clip Plane (for reflection)
        //============================================================
        ConstantBuffer<ClipPlaneCB>     m_clipPlaneCb;      /// b6 クリップ平面
        ClipPlaneCB                     m_clipPlaneData{};  /// クリップ平面データ
        bool                            m_clipPlaneEnabled = false; /// クリップ平面有効フラグ

    private:
		static constexpr float kDefaultClearColor[4] = { 0.2f, 1.0f, 0.2f, 1.0f };  // デフォルトのクリアカラー
    };

} // namespace Engine
