#pragma once

#include <cstdint>
#include <DirectXMath.h>

#include "Engine/Math/Vector4.h"

namespace Engine {

    static constexpr std::uint32_t kMaterialFlagUseTexture  = 1u << 0;
    static constexpr std::uint32_t kMaterialFlagAlphaTest   = 1u << 1;

    static constexpr std::uint32_t kLightFlagEnabled        = 1u << 0;
    static constexpr std::uint32_t kLightFlagCastShadow     = 1u << 1;

    // ライト数の最大値
    static constexpr std::uint32_t kMaxDirectionalLights    = 4;
    static constexpr std::uint32_t kMaxPointLights          = 16;
    static constexpr std::uint32_t kMaxSpotLights           = 16;

    // スキニング
    static constexpr std::uint32_t kMaxBones                = 128;

    struct alignas(16) WorldCB final        { DirectX::XMFLOAT4X4 g_world{}; };
    struct alignas(16) ViewCB final         { DirectX::XMFLOAT4X4 g_view{}; };
    struct alignas(16) ProjectionCB final   { DirectX::XMFLOAT4X4 g_projection{}; };

    struct alignas(16) MaterialParams final {
        Vector4 m_baseColor { 1,1,1,1 };
        Vector4 m_ambient   { 1,1,1,1 };
        Vector4 m_specular  { 0,0,0,0 };
        Vector4 m_emissive  { 0,0,0,0 };

        float m_shininess       = 0.0f;
        float m_alphaCutoff     = 0.5f;
        std::uint32_t m_flags   = 0;
        std::uint32_t m_pad0    = 0;
    };

    struct alignas(16) MaterialCB final { MaterialParams g_material{}; };

    /// @brief 方向性ライト（シェーダー用構造体）
    struct alignas(16) DirectionalLight final {
        float m_directionX      = 0.0f;
        float m_directionY      = -1.0f;
        float m_directionZ      = 0.0f;
        std::uint32_t m_flags   = 0;

        Vector4 m_diffuse{ 1,1,1,1 };
        Vector4 m_ambient{ 0.2f,0.2f,0.2f,1.0f };
    };

    /// @brief 点光源（シェーダー用構造体）
    struct alignas(16) PointLight final {
        float m_positionX   = 0.0f;         /// 位置X
        float m_positionY   = 0.0f;         /// 位置Y
        float m_positionZ   = 0.0f;         /// 位置Z
        float m_range       = 10.0f;        /// 到達距離

        Vector4 m_diffuse{ 1,1,1,1 };       /// 拡散色（RGB）と強度（A）

        float m_constantAtten   = 1.0f;     /// 定数減衰
        float m_linearAtten     = 0.09f;    /// 線形減衰
        float m_quadraticAtten  = 0.032f;   /// 二次減衰
        std::uint32_t m_flags   = 0;        /// フラグ
    };

    /// @brief スポットライト（シェーダー用構造体）
    struct alignas(16) SpotLight final {
        float m_positionX   = 0.0f;         /// 位置X
        float m_positionY   = 0.0f;         /// 位置Y
        float m_positionZ   = 0.0f;         /// 位置Z
        float m_range       = 10.0f;        /// 到達距離

        float m_directionX      = 0.0f;     /// 方向X
        float m_directionY      = -1.0f;    /// 方向Y
        float m_directionZ      = 0.0f;     /// 方向Z
        float m_innerCosAngle   = 0.866f;   /// cos(内側角度)

        Vector4 m_diffuse{ 1,1,1,1 };       /// 拡散色（RGB）と強度（A）

        float m_outerCosAngle   = 0.707f;   /// cos(外側角度)
        float m_constantAtten   = 1.0f;     /// 定数減衰
        float m_linearAtten     = 0.09f;    /// 線形減衰
        float m_quadraticAtten  = 0.032f;   /// 二次減衰

        std::uint32_t m_flags   = 0;        /// フラグ
        std::uint32_t m_pad0    = 0;
        std::uint32_t m_pad1    = 0;
        std::uint32_t m_pad2    = 0;
    };

    struct alignas(16) LightCB final { DirectionalLight g_light{}; };

    /// @brief 拡張ライト定数バッファ（複数ライト対応）
    struct alignas(16) ExtendedLightCB final {
        DirectionalLight g_directionalLights[kMaxDirectionalLights]{};
        PointLight g_pointLights[kMaxPointLights]{};
        SpotLight g_spotLights[kMaxSpotLights]{};

        std::uint32_t g_directionalLightCount = 0;
        std::uint32_t g_pointLightCount = 0;
        std::uint32_t g_spotLightCount = 0;
        std::uint32_t g_pad0 = 0;
    };

    /// @brief HDR設定定数バッファ
    struct alignas(16) HDRSettingsCB final {
        float g_exposure        = 1.0f;     /// 露出
        float g_gamma           = 2.2f;     /// ガンマ値
        float g_displayNits     = 80.0f;    /// ディスプレイ基準輝度（nits）
        float g_paperWhiteNits  = 200.0f;   /// 紙白輝度（nits）

        float g_maxLuminance    = 1000.0f;  /// 最大輝度
        float g_minLuminance    = 0.001f;   /// 最小輝度
        std::uint32_t g_tonemapMode = 0;    /// トーンマップモード（0=なし、1=Reinhard、2=ACES）
        std::uint32_t g_pad0        = 0;
    };

    /// @brief ボリューメトリックライティング設定
    struct alignas(16) VolumetricLightingCB final {
        float g_scatteringIntensity         = 0.5f;     /// 散乱強度
        float g_fogDensity                  = 0.01f;    /// フォグ密度
        std::uint32_t g_sampleCount         = 64;       /// サンプル数
        std::uint32_t g_enableVolumetric    = 0;        /// 有効フラグ

        float g_absorptionR = 0.0f;          /// 吸収係数R
        float g_absorptionG = 0.0f;          /// 吸収係数G
        float g_absorptionB = 0.0f;          /// 吸収係数B
        float g_anisotropy  = 0.5f;          /// 異方性係数（-1〜1）
    };

    // b5 : Bones
    struct alignas(16) BonesCB final {
        DirectX::XMFLOAT4X4 g_bones[kMaxBones]{};
    };

    // b6 : Clip Plane (for planar reflection)
    struct alignas(16) ClipPlaneCB final {
        DirectX::XMFLOAT4 g_clipPlane{ 0.0f, 1.0f, 0.0f, 0.0f }; ///< クリップ平面（x,y,z=法線、w=距離）
        float g_clipPlaneEnabled = 0.0f;   ///< クリップ平面有効フラグ（0.0 or 1.0）
        float g_pad0 = 0.0f;
        float g_pad1 = 0.0f;
        float g_pad2 = 0.0f;
    };

    //=========================================================================
    // Terrain Blend Material (b7)
    // 地形用マルチテクスチャブレンドのための定数バッファ
    //=========================================================================

    /// @brief 地形テクスチャレイヤーの最大数
    static constexpr std::uint32_t kMaxTerrainLayers = 4;

    /// @brief 地形ブレンドマテリアル用パラメータ
    /// @note  複数テクスチャのブレンド比率や、UVスケールを制御する
    struct alignas(16) TerrainBlendParams final {
        /// @brief 各レイヤーのブレンドウェイト（RGBA = Layer0,1,2,3）
        Vector4 m_blendWeights{ 1.0f, 0.0f, 0.0f, 0.0f };

        /// @brief 各レイヤーのUVスケール（タイリング倍率）
        Vector4 m_uvScales{ 10.0f, 10.0f, 10.0f, 10.0f };

        /// @brief 各レイヤーの色調補正（Layer 0～3の順）
        Vector4 m_layerTints[kMaxTerrainLayers] = {
            { 1.0f, 1.0f, 1.0f, 1.0f },  ///< Layer 0（例：草）
            { 1.0f, 1.0f, 1.0f, 1.0f },  ///< Layer 1（例：土）
            { 1.0f, 1.0f, 1.0f, 1.0f },  ///< Layer 2（例：砂）
            { 1.0f, 1.0f, 1.0f, 1.0f },  ///< Layer 3（例：岩）
        };

        /// @brief 有効なレイヤー数（1～4）
        std::uint32_t m_activeLayerCount = 1;

        /// @brief フラグ（予約）
        std::uint32_t m_flags = 0;

        std::uint32_t m_pad0 = 0;
        std::uint32_t m_pad1 = 0;
    };

    /// @brief 地形ブレンドマテリアル定数バッファ（b7）
    struct alignas(16) TerrainBlendCB final {
        TerrainBlendParams g_terrainBlend{};
    };

    inline DirectX::XMFLOAT4X4 MakeIdentityMatrix() {
        DirectX::XMFLOAT4X4 m{};
        DirectX::XMStoreFloat4x4(&m, DirectX::XMMatrixIdentity());
        return m;
    }

} // namespace Engine
