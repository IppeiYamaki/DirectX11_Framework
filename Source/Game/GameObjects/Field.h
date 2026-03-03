/// @file   Field.h
/// @brief  地形（フィールド）のGameObject派生クラス
/// @note   Diamond-Squareアルゴリズムで高さマップを生成し、地形をレンダリングする
#pragma once

#include "Engine/Scene/GameObject.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector4.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <memory>
#include <random>

namespace Engine {
    class Material;
    class Mesh;
    class Texture;
    class RenderSystem;
    class AssetManager;
}

namespace Game {

    /// @brief 地形（フィールド）用のGameObject派生クラス
    /// @note  Diamond-Squareアルゴリズムで高さマップを生成し、地形メッシュを作成する
    class Field : public Engine::GameObject {
    public:
        /// @brief 頂点構造体（位置/法線/色/UV）
        struct Vertex {
            Engine::Vector3 m_pos;      ///< 位置
            Engine::Vector3 m_normal;   ///< 法線
            Engine::Vector4 m_color;    ///< 色
            Engine::Vector2 m_uv;       ///< テクスチャ座標
        };

        /// @brief コンストラクタ
        /// @param name オブジェクト名（デフォルト: "Field"）
        explicit Field(const std::string& name = "Field");

        /// @brief デストラクタ
        ~Field() override;

        //============================================================
        // ライフサイクルオーバーライド
        //============================================================

        /// @brief 初期化処理
        void Initialize() override;

        /// @brief 毎フレームの更新処理
        /// @param deltaTime 前フレームからの経過時間（秒）
        void Update(float deltaTime) override;

        /// @brief 描画処理
        void Render() override;

        /// @brief 破棄処理
        void OnDestroy() override;

        //============================================================
        // セットアップ（D3Dリソース初期化）
        //============================================================

        /// @brief Direct3D 11リソースを初期化
        /// @param device D3D11デバイス
        /// @param renderSystem レンダーシステム
        /// @param assetManager アセットマネージャー（テクスチャ読み込み用）
        /// @return 成功すればtrue
        bool Setup(ID3D11Device* device, Engine::RenderSystem* renderSystem, Engine::AssetManager* assetManager);

        //============================================================
        // 地形設定
        //============================================================

        /// @brief グリッドサイズを設定（2^n + 1 推奨）
        /// @param size グリッドサイズ（例: 129, 257, 513）
        /// @note  変更後にRegenerate()を呼び出す必要があります
        void SetGridSize(int size);

        /// @brief グリッドサイズを取得
        /// @return 現在のグリッドサイズ
        [[nodiscard]] int GetGridSize() const;

        /// @brief セルサイズを設定（1マスの大きさ）
        /// @param size セルサイズ（正の値である必要があります）
        /// @note  変更後にRegenerate()を呼び出す必要があります
        void SetCellSize(float size);

        /// @brief セルサイズを取得
        /// @return 現在のセルサイズ
        [[nodiscard]] float GetCellSize() const;

        /// @brief 高さスケールを設定
        /// @param amp 高さの振幅（スケール）
        /// @note  変更後にRegenerate()を呼び出す必要があります
        void SetHeightAmplitude(float amp);

        /// @brief 高さスケールを取得
        /// @return 現在の高さスケール
        [[nodiscard]] float GetHeightAmplitude() const;

        /// @brief 荒さパラメータを設定（0.4～0.7くらいが使いやすい）
        /// @param roughness 荒さパラメータ
        /// @note  変更後にRegenerate()を呼び出す必要があります
        void SetRoughness(float roughness);

        /// @brief 荒さパラメータを取得
        /// @return 現在の荒さパラメータ
        [[nodiscard]] float GetRoughness() const;

        /// @brief 地形を再生成
        /// @param seed 乱数シード（デフォルト: ランダム）
        void Regenerate(unsigned int seed = 0);

        //============================================================
        // 地形情報取得
        //============================================================

        /// @brief 指定位置の高さを取得
        /// @param x X座標（ワールド座標）
        /// @param z Z座標（ワールド座標）
        /// @return 高さ（Y座標）、範囲外の場合は0.0f
        [[nodiscard]] float GetHeightAt(float x, float z) const;

        /// @brief マテリアルを設定
        /// @param material 使用するマテリアル
        void SetMaterial(const std::shared_ptr<Engine::Material>& material);

        /// @brief メッシュを取得
        /// @return 生成された地形メッシュ
        [[nodiscard]] const std::shared_ptr<Engine::Mesh>& GetMesh() const;

    private:
        //============================================================
        // 内部処理
        //============================================================

        /// @brief Diamond-Squareで高さマップ生成
        /// @param heightmap 出力先の高さマップ（gridSize * gridSize）
        /// @param gridSize グリッドサイズ
        /// @param roughness 荒さパラメータ
        /// @param seed 乱数シード
        void GenerateHeightmap_DiamondSquare(
            std::vector<float>& heightmap,
            int gridSize,
            float roughness,
            unsigned int seed
        );

        /// @brief 高さマップからメッシュを生成
        /// @return 成功すればtrue
        bool CreateMeshFromHeightmap();

        /// @brief 法線を計算
        /// @param vertices 頂点配列
        /// @param gridSize グリッドサイズ
        void CalculateNormals(std::vector<Vertex>& vertices, int gridSize);

    private:
        // D3D11リソース
        ID3D11Device* m_device = nullptr;
        Engine::RenderSystem* m_renderSystem = nullptr;
        Engine::AssetManager* m_assetManager = nullptr;

        // メッシュとマテリアル
        std::shared_ptr<Engine::Mesh> m_mesh;
        std::shared_ptr<Engine::Material> m_material;

        // 高さマップ
        std::vector<float> m_heightmap;

        // 地形設定
        int   m_gridSize = 257;     ///< 2^n + 1 推奨（例: 129, 257, 513）
        float m_cellSize = 1.0f;    ///< 1マスの大きさ
        float m_heightAmp = 12.0f;  ///< 高さのスケール
        float m_roughness = 0.55f;  ///< 0.4～0.7くらいが使いやすい

        // 状態フラグ
        bool m_isSetup = false;     ///< セットアップ済みフラグ
    };

} // namespace Game
