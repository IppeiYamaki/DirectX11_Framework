#pragma once

#include <d3d11.h>

#include "Engine/Graphics/Mesh.h"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"

namespace Engine {

    /// @brief 位置/法線/色/UVの標準的な頂点構造体
    struct VertexPosNormColorUv final {
        Vector3 m_pos;
        Vector3 m_normal;
        Vector4 m_color;
        Vector2 m_uv;
    };

    /// @brief 位置/法線/タンジェント/ビタジェント/UVの頂点構造体
    struct VertexPosNormTangentBitangentUv final {
        Vector3 m_pos;
        Vector3 m_normal;
        Vector3 m_tangent;
        Vector3 m_bitangent;
        Vector2 m_uv;
    };

    class MeshPrimitives final {
    public:
        MeshPrimitives() = delete;

		/// @brief Quadの生成
        /// @param  device D3D11デバイス
        /// @param  outMesh 生成されたメッシュの出力先
        /// @param  width Quadの幅（X方向サイズ）
        /// @param  height Quadの高さ（Y方向サイズ）
		/// @return 成功ならtrue
        static bool CreateQuad      (ID3D11Device* device, Mesh& outMesh, float width   = 1.0f,  float height           = 1.0f);
		/// @brief Cubeの生成
		/// @param  device D3D11デバイス
        /// @param  outMesh 生成されたメッシュの出力先
        /// @param  sx Cubeの幅（X方向サイズ）
        /// @param  sy Cubeの高さ（Y方向サイズ）
        /// @param  sz Cubeの奥行き（Z方向サイズ）
        /// @return 成功ならtrue
        static bool CreateCube      (ID3D11Device* device, Mesh& outMesh, float sx      = 1.0f,  float sy               = 1.0f,  float sz       = 1.0f);
		/// @brief PlaneGridの生成
		/// @param  device D3D11デバイス
        /// @param  outMesh 生成されたメッシュの出力先
        /// @param  width PlaneGridの幅（X方向サイズ）
        /// @param  depth PlaneGridの奥行き（Z方向サイズ）
        /// @param  gridX PlaneGridのX方向分割数
        /// @param  gridZ PlaneGridのY方向分割数
        /// @return 成功ならtrue
        static bool CreatePlaneGrid (ID3D11Device* device, Mesh& outMesh, float width   = 10.0f, float depth            = 10.0f, int   gridX    = 10,   int gridZ               = 10);

		/// @brief Sphereの生成（UV展開版）
		/// @param  device D3D11デバイス
		/// @param  outMesh 生成されたメッシュの出力先
        /// @param  radius Sphereの半径
        /// @param  slices Sphereのスライス数
        /// @param  stacks Sphereのスタック数
        /// @return 成功ならtrue
        static bool CreateSphereUv  (ID3D11Device* device, Mesh& outMesh, float radius  = 0.5f,  int   slices           = 32,    int   stacks   = 16);
		/// @brief Sphereの生成（正二十面体分割版）
		/// @param  device D3D11デバイス
		/// @param  outMesh 生成されたメッシュの出力先
        /// @param  radius Sphereの半径
        /// @param  subdivisions Sphereの分割数
        /// @return 成功ならtrue
        static bool CreateSphereIco (ID3D11Device* device, Mesh& outMesh, float radius  = 0.5f,  int   subdivisions     = 2);

		/// @brief Capsuleの生成
		/// @param  device D3D11デバイス
		/// @param  outMesh 生成されたメッシュの出力先
        /// @param  radius Capsuleの半径
        /// @param  height Capsuleの高さ
        /// @param  slices Capsuleのスライス数
        /// @param  stacksHemisphere Capsuleの半球部分のスタック数
        /// @param  stacksCylinder Capsuleの円柱部分のスタック数
        /// @return 成功ならtrue
        static bool CreateCapsule   (ID3D11Device* device, Mesh& outMesh, float radius  = 0.5f,  float height           = 2.0f,  int   slices   = 32,   int stacksHemisphere    = 8,    int stacksCylinder = 4);

        /// @brief フィールド用グリッドメッシュの生成（Perlinノイズによる自然地形）
        /// @param  device D3D11デバイス
        /// @param  outMesh 生成されたメッシュの出力先
        /// @param  width フィールドの幅（X方向サイズ）
        /// @param  depth フィールドの奥行き（Z方向サイズ）
        /// @param  gridSize フィールドの分割数
        /// @param  amplitude 地形の高さスケール（デフォルト: 10.0）
        /// @param  frequency ノイズの周波数（デフォルト: 0.05、小さいほど緩やかな地形）
        /// @param  octaves フラクタルノイズのオクターブ数（デフォルト: 4）
        /// @param  seed ノイズのシード値（デフォルト: 0）
        /// @return 成功ならtrue
        static bool CreateFieldGrid (ID3D11Device* device, Mesh& outMesh, float width = 100.0f, float depth = 100.0f, int gridSize = 10,
                                      float amplitude = 10.0f, float frequency = 0.05f, int octaves = 4, std::uint32_t seed = 0);
    };

} // namespace Engine
