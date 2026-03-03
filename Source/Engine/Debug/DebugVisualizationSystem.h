/// @file   DebugVisualizationSystem.h
/// @brief  デバッグ開発者モードの可視化システム
#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>

#include "Engine/Scene/GameObject.h"

#include "Engine/Scene/Light.h"


namespace Engine {

    class RenderSystem;
    class Scene;
    class Texture;
    class GraphicsDevice;
    class LightSystem;
    class VertexShader;
    class PixelShader;
    class InputLayout;
    class Mesh;
    class GameObject;
    struct SceneContext;

    /// @brief デバッグ可視化の種類
    enum class DebugVisualizationType {
        None = 0,               ///< 可視化なし
        Outline = 1,            ///< アウトライン表示
        LightIcon = 2,          ///< ライトアイコン表示（2Dビルボード）【非推奨: LightSphereを使用】
        LightSphere = 4,        ///< ライト位置に3D球体表示（MeshType::Sphere）
        All = 5                 ///< 全て表示（アウトライン + ライト球体）
    };

    /// @brief デバッグ可視化システム
    /// @note  0キーで開発者モードON/OFF、GameObjectのアウトラインやライト位置の3D球体を表示
    class DebugVisualizationSystem final {
    public:
        DebugVisualizationSystem() = default;
        ~DebugVisualizationSystem();

        DebugVisualizationSystem(const DebugVisualizationSystem&) = delete;
        DebugVisualizationSystem& operator=(const DebugVisualizationSystem&) = delete;

        //============================================================
        // Lifecycle
        //============================================================

        /// @brief  初期化
        /// @param  graphicsDevice グラフィックスデバイス
        /// @param  renderSystem RenderSystem参照
        /// @return 成功ならtrue
        [[nodiscard]] bool Initialize(GraphicsDevice* graphicsDevice, RenderSystem* renderSystem);

        /// @brief 終了処理
        void Finalize();

        //============================================================
        // Frame
        //============================================================

        /// @brief 毎フレーム更新（キー入力チェック）
        /// @param deltaTime フレーム経過時間
        /// @param ctx SceneContext参照（デバッグライトマーカー生成/削除に使用）
        /// @param lightSystem LightSystem参照（デバッグライトマーカー生成に使用）
        void Update(float deltaTime, SceneContext& ctx, LightSystem* lightSystem);

        /// @brief デバッグ可視化を描画
        /// @param scene Scene参照
        /// @param lightSystem LightSystem参照
        /// @param viewMatrix ビュー行列
        /// @param projMatrix 射影行列
        void Render(Scene* scene, LightSystem* lightSystem,
                    const DirectX::XMFLOAT4X4& viewMatrix,
                    const DirectX::XMFLOAT4X4& projMatrix);

        //============================================================
        // Mode Control
        //============================================================

        /// @brief  デバッグモードを有効/無効切替
        /// @param  enabled 有効にする場合true
        void SetEnabled(bool enabled);

        /// @brief  デバッグモードが有効か確認
        /// @return 有効ならtrue
        [[nodiscard]] bool IsEnabled() const;

        /// @brief  デバッグモードをトグル
        void ToggleEnabled();

        /// @brief  可視化タイプを設定
        /// @param  type 可視化タイプ
        void SetVisualizationType(DebugVisualizationType type);

        /// @brief  可視化タイプを取得
        /// @return 可視化タイプ
        [[nodiscard]] DebugVisualizationType GetVisualizationType() const;

        //============================================================
        // Outline Settings
        //============================================================

        /// @brief  アウトライン色を設定（蛍光緑のネオン風）
        /// @param  color RGB色（0〜1）
        void SetOutlineColor(const DirectX::XMFLOAT3& color);

        /// @brief  アウトライン色を取得
        /// @return アウトライン色
        [[nodiscard]] DirectX::XMFLOAT3 GetOutlineColor() const;

        /// @brief  アウトライン太さを設定
        /// @param  thickness 太さ
        void SetOutlineThickness(float thickness);

        /// @brief  アウトライン太さを取得
        /// @return アウトライン太さ
        [[nodiscard]] float GetOutlineThickness() const;

        //============================================================
        // Light Icon Settings【非推奨: Light Sphere Settingsを使用】
        //============================================================

        /// @brief  ライトアイコンサイズを設定【非推奨: SetLightSphereSizeを使用】
        /// @param  size サイズ
        void SetLightIconSize(float size);

        /// @brief  ライトアイコンサイズを取得【非推奨: GetLightSphereSizeを使用】
        /// @return サイズ
        [[nodiscard]] float GetLightIconSize() const;

        /// @brief  DirectionalLightの表示位置を設定【非推奨: SetDirectionalLightSpherePositionを使用】
        /// @param  position 表示位置
        /// @note   SetDirectionalLightSpherePositionと同じ内部データを操作します
        void SetDirectionalLightIconPosition(const DirectX::XMFLOAT3& position);

        /// @brief  DirectionalLightの表示位置を取得【非推奨: GetDirectionalLightSpherePositionを使用】
        /// @return 表示位置
        /// @note   GetDirectionalLightSpherePositionと同じ内部データを返します
        [[nodiscard]] DirectX::XMFLOAT3 GetDirectionalLightIconPosition() const;

        //============================================================
        // Light Sphere Settings (3D球体表示)
        //============================================================

        /// @brief  ライト球体サイズを設定
        /// @param  size サイズ（半径）
        void SetLightSphereSize(float size);

        /// @brief  ライト球体サイズを取得
        /// @return サイズ（半径）
        [[nodiscard]] float GetLightSphereSize() const;

        /// @brief  ライト球体表示を有効/無効切替
        /// @param  enabled 有効にする場合true
        void SetLightSphereEnabled(bool enabled);

        /// @brief  ライト球体表示が有効か確認
        /// @return 有効ならtrue
        [[nodiscard]] bool IsLightSphereEnabled() const;

        /// @brief  DirectionalLight球体の表示位置を設定
        /// @param  position 表示位置
        void SetDirectionalLightSpherePosition(const DirectX::XMFLOAT3& position);

        /// @brief  DirectionalLight球体の表示位置を取得
        /// @return 表示位置
        [[nodiscard]] DirectX::XMFLOAT3 GetDirectionalLightSpherePosition() const;

        /// @brief  初期化済みか確認
        /// @return 初期化済みならtrue
        [[nodiscard]] bool IsInitialized() const;

        //============================================================
        // Debug Object Management (Prefab-based Light Markers)
        //============================================================

        /// @brief  デバッグライトマーカーを生成（Prefabベース）
        /// @param  ctx SceneContext参照
        /// @param  lightSystem LightSystem参照
        /// @note   デバッグモード有効時に呼び出してライト位置にマーカーを生成
        void SpawnDebugLightMarkers(SceneContext& ctx, LightSystem* lightSystem);

        /// @brief  デバッグライトマーカーを全て削除
        /// @param  ctx SceneContext参照
        /// @note   デバッグモード無効時に呼び出して即座に削除
        void ClearDebugLightMarkers(SceneContext& ctx);

        /// @brief  デバッグライトマーカーを更新（位置同期）
        /// @param  lightSystem LightSystem参照
        /// @note   ライト位置が変更された場合にマーカー位置を同期
        void UpdateDebugLightMarkerPositions(LightSystem* lightSystem);

        /// @brief  デバッグライトマーカーの数を取得
        /// @return 現在のマーカー数
        [[nodiscard]] std::size_t GetDebugLightMarkerCount() const;

        /// @brief  Prefabベースのライトマーカーを使用するか設定
        /// @param  use trueでPrefabベース、falseで従来の直接描画
        void SetUsePrefabLightMarkers(bool use);

        /// @brief  Prefabベースのライトマーカーを使用しているか確認
        /// @return Prefabベースならtrue
        [[nodiscard]] bool IsUsingPrefabLightMarkers() const;

    private:
        /// @brief アウトライン用シェーダーを初期化
        /// @return 成功ならtrue
        bool InitializeOutlineResources();

        /// @brief ビルボード用シェーダーを初期化
        /// @return 成功ならtrue
        bool InitializeBillboardResources();

        /// @brief ライトアイコンテクスチャを読み込み
        /// @return 成功ならtrue
        bool LoadLightIconTextures();

        /// @brief ライト球体用リソースを初期化
        /// @return 成功ならtrue
        bool InitializeLightSphereResources();

        /// @brief アウトライン描画
        /// @param scene Scene参照
        /// @param viewMatrix ビュー行列
        /// @param projMatrix 射影行列
        void RenderOutlines(Scene* scene,
                           const DirectX::XMFLOAT4X4& viewMatrix,
                           const DirectX::XMFLOAT4X4& projMatrix);

        /// @brief ライトアイコン描画
        /// @param lightSystem LightSystem参照
        /// @param viewMatrix ビュー行列
        /// @param projMatrix 射影行列
        void RenderLightIcons(LightSystem* lightSystem,
                             const DirectX::XMFLOAT4X4& viewMatrix,
                             const DirectX::XMFLOAT4X4& projMatrix);

        /// @brief ライト位置に3D球体を描画
        /// @param lightSystem LightSystem参照
        /// @param viewMatrix ビュー行列
        /// @param projMatrix 射影行列
        void RenderLightSpheres(LightSystem* lightSystem,
                               const DirectX::XMFLOAT4X4& viewMatrix,
                               const DirectX::XMFLOAT4X4& projMatrix);

        /// @brief ビルボード頂点バッファを作成
        void CreateBillboardVertexBuffer();

    private:
        GraphicsDevice* m_graphicsDevice = nullptr;     ///< グラフィックスデバイス（借用）
        RenderSystem* m_renderSystem = nullptr;          ///< RenderSystem（借用）
        bool m_isInitialized = false;                    ///< 初期化済みフラグ

        // モード状態
        bool m_isEnabled = false;                        ///< デバッグモード有効フラグ
        bool m_wasEnabledLastFrame = false;              ///< 前フレームのデバッグモード状態（変化検出用）
        DebugVisualizationType m_visualizationType = DebugVisualizationType::All;

        // アウトライン設定
        DirectX::XMFLOAT3 m_outlineColor{0.2f, 1.0f, 0.2f};  ///< 蛍光緑
        float m_outlineThickness = 0.03f;                      ///< アウトライン太さ

        // ライトアイコン設定
        float m_lightIconSize = 1.0f;                    ///< アイコンサイズ
        DirectX::XMFLOAT3 m_directionalLightIconPos{0.0f, 10.0f, 0.0f}; ///< DirectionalLightアイコン表示位置

        // アウトライン用リソース
        std::unique_ptr<VertexShader> m_outlineVS;       ///< アウトライン頂点シェーダー
        std::unique_ptr<PixelShader> m_outlinePS;        ///< アウトラインピクセルシェーダー
        std::unique_ptr<InputLayout> m_outlineInputLayout; ///< アウトライン入力レイアウト

        // ビルボード用リソース
        std::unique_ptr<VertexShader> m_billboardVS;     ///< ビルボード頂点シェーダー
        std::unique_ptr<PixelShader> m_billboardPS;      ///< ビルボードピクセルシェーダー
        std::unique_ptr<InputLayout> m_billboardInputLayout; ///< ビルボード入力レイアウト
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_billboardVertexBuffer; ///< ビルボード頂点バッファ
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_billboardConstantBuffer; ///< ビルボード定数バッファ

        // 矢印（方向表示）用リソース
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_arrowVertexBuffer;  ///< 矢印頂点バッファ
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_arrowConstantBuffer; ///< 矢印定数バッファ

        // ライトアイコンテクスチャ
        std::unique_ptr<Texture> m_pointLightIcon;       ///< PointLightアイコン
        std::unique_ptr<Texture> m_directionalLightIcon; ///< DirectionalLightアイコン
        std::unique_ptr<Texture> m_spotLightIcon;        ///< SpotLightアイコン

        // ライト球体用リソース（3D球体表示）
        std::unique_ptr<VertexShader> m_unlitVS;         ///< アンリット頂点シェーダー
        std::unique_ptr<PixelShader> m_unlitPS;          ///< アンリットピクセルシェーダー
        std::unique_ptr<InputLayout> m_unlitInputLayout; ///< アンリット入力レイアウト
        std::unique_ptr<Mesh> m_sphereMesh;              ///< 球体メッシュ
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_unlitConstantBuffer; ///< アンリット定数バッファ
        float m_lightSphereSize = 0.3f;                  ///< ライト球体サイズ（半径）
        bool m_lightSphereEnabled = true;                ///< ライト球体表示有効フラグ

        // アウトライン定数バッファ
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_outlineConstantBuffer; ///< アウトライン定数バッファ

        // ラスタライザーステート
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rsCullFront;  ///< 前面カリング
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rsCullNone;   ///< カリングなし

        // ブレンドステート
        Microsoft::WRL::ComPtr<ID3D11BlendState> m_bsAdditive;        ///< 加算合成

        // 深度ステンシルステート
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_dsDepthOff; ///< 深度テストOFF

        // サンプラーステート
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;         ///< テクスチャサンプラー

        // デバッグモード切替キー
        static constexpr int kDebugModeToggleKey = '0';        ///< デバッグモード切替キー（0）

        //============================================================
        // Debug Object Management (Prefab-based Light Markers)
        //============================================================

        /// @brief デバッグ用ライトマーカーGameObjectのリスト（借用ポインタ）
        std::vector<GameObject*> m_debugLightMarkers;

        /// @brief Prefabベースのライトマーカーを使用するかフラグ
        bool m_usePrefabLightMarkers = true;

        /// @brief ライトマーカー情報（ライト-マーカー関連付け）
        struct LightMarkerInfo {
            GameObject* m_marker = nullptr;  ///< マーカーGameObject
            Light* m_light = nullptr;        ///< 関連付けられたライト
        };

        /// @brief ライトマーカー情報のリスト
        std::vector<LightMarkerInfo> m_lightMarkerInfos;
    };

} // namespace Engine
