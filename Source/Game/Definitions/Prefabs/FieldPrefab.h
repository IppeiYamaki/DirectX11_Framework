/// @file   FieldPrefab.h
/// @brief  地形（フィールド）用Prefabクラス
/// @note   MeshType::FieldをMeshRenderer経由で使用し、複数テクスチャのブレンド設定も可能
#pragma once

#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"
#include <array>
#include <string>

namespace Engine { 
    struct SceneContext;
    class GameObject;
}

namespace Game {

    /// @brief 地形（フィールド）Prefab
    /// @note  MeshRenderer + MeshType::Field でフィールドを生成する
    /// 
    /// 使用例：
    /// @code
    /// // 基本的なフィールド生成
    /// FieldPrefab::SpawnDesc desc;
    /// desc.m_position = Engine::Vector3(0, 0, 0);
    /// desc.m_width = 200.0f;
    /// desc.m_depth = 200.0f;
    /// desc.m_gridSize = 64;
    /// auto* field = FieldPrefab::Spawn(ctx, desc);
    /// 
    /// // マルチテクスチャブレンドを使用する場合
    /// FieldPrefab::SpawnDesc descWithBlend;
    /// descWithBlend.m_useMeshRenderer = true;
    /// descWithBlend.m_useTerrainBlend = true;
    /// descWithBlend.m_texturePaths[0] = L"Textures/Terrain/grass.png";
    /// descWithBlend.m_texturePaths[1] = L"Textures/Terrain/dirt.png";
    /// descWithBlend.m_blendWeights = Engine::Vector4(0.6f, 0.4f, 0.0f, 0.0f);
    /// descWithBlend.m_uvScales = Engine::Vector4(20.0f, 15.0f, 10.0f, 10.0f);
    /// auto* blendField = FieldPrefab::Spawn(ctx, descWithBlend);
    /// @endcode
    class FieldPrefab final {
    public:
        /// @brief テクスチャレイヤーの最大数
        static constexpr std::uint32_t kMaxLayers = 4;

        /// @brief 生成パラメータ
        struct SpawnDesc final {
            //============================================================
            // Transform
            //============================================================
            Engine::Vector3 m_position{ 0, 0, 0 };      ///< 初期位置
            Engine::Vector3 m_rotation{ 0, 0, 0 };      ///< 初期回転（度）
            float m_uniformScale = 1.0f;                 ///< 均一スケール

            //============================================================
            // Mesh Parameters
            //============================================================
            float m_width = 100.0f;                      ///< フィールドの幅（X方向）
            float m_depth = 100.0f;                      ///< フィールドの奥行き（Z方向）
            int   m_gridSize = 256;                       ///< グリッド分割数

            //============================================================
            // Perlin Noise Terrain Parameters
            //============================================================
            float m_terrainAmplitude = 5.0f;            ///< 地形の高さスケール
            float m_terrainFrequency = 0.05f;            ///< ノイズ周波数（小さいほど緩やか）
            int   m_terrainOctaves = 100;                  ///< フラクタルノイズのオクターブ数
            std::uint32_t m_terrainSeed = 0;             ///< ノイズシード値（地形バリエーション）

            //============================================================
            // Legacy Field Parameters (for Diamond-Square terrain)
            //============================================================
            bool  m_useLegacyField = false;              ///< レガシーFieldクラスを使用するか
            float m_heightAmplitude = 12.0f;             ///< 高さスケール（レガシー用）
            float m_roughness = 0.55f;                   ///< 荒さパラメータ（レガシー用）
            float m_cellSize = 1.0f;                     ///< セルサイズ（レガシー用）
            int   m_legacyGridSize = 257;                ///< レガシーグリッドサイズ（2^n + 1 推奨）

            //============================================================
            // MeshRenderer Mode
            //============================================================
            bool m_useMeshRenderer = true;               ///< MeshRenderer経由でメッシュを生成するか

            //============================================================
            // Material/Texture Blend Settings
            //============================================================
            bool m_useTerrainBlend = false;              ///< マルチテクスチャブレンドを使用するか

            /// @brief テクスチャパス（Layer 0～3）
            /// @note  空文字列の場合はデフォルトテクスチャを使用
            std::array<std::wstring, kMaxLayers> m_texturePaths{};

            /// @brief 各レイヤーのブレンドウェイト（x,y,z,w = Layer 0,1,2,3）
            /// @note  合計が1.0になるように正規化される
            Engine::Vector4 m_blendWeights{ 1.0f, 0.0f, 0.0f, 0.0f };

            /// @brief 各レイヤーのUVスケール（タイリング倍率）
            Engine::Vector4 m_uvScales{ 10.0f, 10.0f, 10.0f, 10.0f };

            /// @brief 各レイヤーの色調補正
            std::array<Engine::Vector4, kMaxLayers> m_layerTints = {{
                { 1.0f, 1.0f, 1.0f, 1.0f },
                { 1.0f, 1.0f, 1.0f, 1.0f },
                { 1.0f, 1.0f, 1.0f, 1.0f },
                { 1.0f, 1.0f, 1.0f, 1.0f },
            }};

            /// @brief 有効なレイヤー数（1～4）
            std::uint32_t m_activeLayerCount = 1;

            /// @brief 単一テクスチャパス（m_useTerrainBlend=falseの場合に使用）
            std::wstring m_singleTexturePath{};

            //============================================================
            // Constructors
            //============================================================
            SpawnDesc() = default;

            /// @brief 位置のみ指定
            SpawnDesc(const Engine::Vector3& position)
                : m_position(position) {
            }

            /// @brief 位置とサイズ指定
            SpawnDesc(const Engine::Vector3& position, float width, float depth, int gridSize = 64)
                : m_position(position)
                , m_width(width)
                , m_depth(depth)
                , m_gridSize(gridSize) {
            }

            /// @brief レガシーField用コンストラクタ（後方互換性）
            SpawnDesc(const Engine::Vector3& position, int gridSize, float cellSize,
                      float heightAmplitude, float roughness)
                : m_position(position)
                , m_useLegacyField(true)
                , m_heightAmplitude(heightAmplitude)
                , m_roughness(roughness)
                , m_cellSize(cellSize)
                , m_legacyGridSize(gridSize)
                , m_useMeshRenderer(false) {
            }

            //============================================================
            // Builder Pattern Methods
            //============================================================

            /// @brief テクスチャレイヤーを設定
            /// @param layerIndex レイヤーインデックス（0～3）
            /// @param texturePath テクスチャパス
            /// @param blendWeight ブレンドウェイト
            /// @param uvScale UVスケール
            /// @return *this（チェーン呼び出し用）
            SpawnDesc& SetTextureLayer(std::uint32_t layerIndex, const std::wstring& texturePath,
                                       float blendWeight = 1.0f, float uvScale = 10.0f) {
                if (layerIndex < kMaxLayers) {
                    m_texturePaths[layerIndex] = texturePath;
                    switch (layerIndex) {
                        case 0: m_blendWeights.x = blendWeight; m_uvScales.x = uvScale; break;
                        case 1: m_blendWeights.y = blendWeight; m_uvScales.y = uvScale; break;
                        case 2: m_blendWeights.z = blendWeight; m_uvScales.z = uvScale; break;
                        case 3: m_blendWeights.w = blendWeight; m_uvScales.w = uvScale; break;
                    }
                    if (layerIndex >= m_activeLayerCount) {
                        m_activeLayerCount = layerIndex + 1;
                    }
                }
                return *this;
            }

            /// @brief マルチテクスチャブレンドを有効化
            /// @return *this（チェーン呼び出し用）
            SpawnDesc& EnableTerrainBlend() {
                m_useTerrainBlend = true;
                m_useMeshRenderer = true;
                return *this;
            }

            /// @brief テクスチャレイヤーを一括設定
            /// @param texturePaths テクスチャパスのリスト
            /// @param defaultBlend デフォルトのブレンドウェイト
            /// @param defaultUv デフォルトのUVスケール
            /// @return *this（チェーン呼び出し用）
            SpawnDesc& SetTextures(std::initializer_list<std::wstring> texturePaths,
                                   float defaultBlend = 1.0f,
                                   float defaultUv = 10.0f) {
                const auto count = static_cast<std::uint32_t>(std::min<std::size_t>(texturePaths.size(), kMaxLayers));
                if (count <= 1U) {
                    m_useTerrainBlend = false;
                    m_activeLayerCount = 1U;
                    m_singleTexturePath = count == 1U ? *texturePaths.begin() : std::wstring{};
                    return *this;
                }

                m_useTerrainBlend = true;
                m_useMeshRenderer = true;
                m_activeLayerCount = count;

                std::uint32_t index = 0;
                for (const auto& path : texturePaths) {
                    m_texturePaths[index] = path;
                    SetTextureLayer(index, path, defaultBlend, defaultUv);
                    ++index;
                    if (index >= count) break;
                }
                return *this;
            }
        };

        /// @brief Fieldを生成してSceneに追加
        /// @param ctx Scene利用コンテキスト情報
        /// @param desc 生成パラメータ
        /// @return 生成されたGameObject
        static Engine::GameObject* Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc);

    private:
        /// @brief MeshRendererを使用したフィールド生成
        static Engine::GameObject* SpawnWithMeshRenderer(Engine::SceneContext& ctx, const SpawnDesc& desc);

        /// @brief レガシーFieldクラスを使用したフィールド生成（後方互換性）
        static Engine::GameObject* SpawnLegacyField(Engine::SceneContext& ctx, const SpawnDesc& desc);
    };

} // namespace Game
