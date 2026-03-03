#pragma once

#include <d3d11.h>

#include "Engine/Graphics/Mesh.h"

namespace Engine {

    /// @brief プリミティブ種別
    enum class MeshType {
        Quad,
        Cube,
        Plane,
        Sphere,
        Capsule,
        Field,      ///< フィールド用グリッドメッシュ
    };

    /// @brief �����p�����[�^�i�K�v�Ȏ������g���j
    /// 
    /// @note Create(device, mesh, MeshType) �� �g�f�t�H���g�l�h �ō��
    /// @note �ׂ����ς������ꍇ�� Create(device, mesh, desc) ���g��
    struct MeshCreateDesc final {
        MeshType m_type = MeshType::Quad;

        // Quad / Plane / Cube �T�C�Y�iCube��XYZ�Ɏg���j
        float       m_width             = 1.0f;
        float       m_height            = 1.0f;
        float       m_depth             = 1.0f;

        // Plane grid
        int         m_gridX             = 10;
        int         m_gridZ             = 10;

        // Sphere
        enum class SphereKind { Uv, Ico };
        SphereKind  m_sphereKind        = SphereKind::Uv;
        float       m_radius            = 0.5f;
        int         m_slices            = 32;
        int         m_stacks            = 16;
        int         m_subdivisions      = 2;

        // Capsule
        float       m_capsuleHeight     = 2.0f;
        int         m_stacksHemisphere  = 8;
        int         m_stacksCylinder    = 4;

        // Field（フィールド用グリッド）
        int         m_fieldGridSize     = 10;   ///< フィールドの分割数
        float       m_fieldAmplitude    = 10.0f; ///< 地形の高さスケール
        float       m_fieldFrequency    = 0.05f; ///< ノイズ周波数
        int         m_fieldOctaves      = 4;     ///< フラクタルノイズのオクターブ数
        std::uint32_t m_fieldSeed       = 0;     ///< ノイズシード値

        static MeshCreateDesc Quad(float w = 1.0f, float h = 1.0f) {
            MeshCreateDesc d{}; d.m_type = MeshType::Quad; d.m_width = w; d.m_height = h; return d;
        }
        static MeshCreateDesc Cube(float s = 1.0f) {
            MeshCreateDesc d{}; d.m_type = MeshType::Cube; d.m_width = s; d.m_height = s; d.m_depth = s; return d;
        }
        static MeshCreateDesc Plane(float w = 10.0f, float d = 10.0f, int gx = 10, int gz = 10) {
            MeshCreateDesc x{}; x.m_type = MeshType::Plane; x.m_width = w; x.m_depth = d; x.m_gridX = gx; x.m_gridZ = gz; return x;
        }
        static MeshCreateDesc SphereUv(float r = 0.5f, int slices = 32, int stacks = 16) {
            MeshCreateDesc d{}; d.m_type = MeshType::Sphere; d.m_sphereKind = SphereKind::Uv; d.m_radius = r; d.m_slices = slices; d.m_stacks = stacks; return d;
        }
        static MeshCreateDesc SphereIco(float r = 0.5f, int sub = 2) {
            MeshCreateDesc d{}; d.m_type = MeshType::Sphere; d.m_sphereKind = SphereKind::Ico; d.m_radius = r; d.m_subdivisions = sub; return d;
        }
        static MeshCreateDesc Capsule(float r = 0.5f, float h = 2.0f, int slices = 32, int hemi = 8, int cyl = 4) {
            MeshCreateDesc d{}; d.m_type = MeshType::Capsule; d.m_radius = r; d.m_capsuleHeight = h; d.m_slices = slices; d.m_stacksHemisphere = hemi; d.m_stacksCylinder = cyl; return d;
        }
        /// @brief フィールド用グリッドメッシュの生成パラメータを作成（Perlinノイズ地形）
        /// @param width フィールドの幅（X方向サイズ）
        /// @param depth フィールドの奥行き（Z方向サイズ）
        /// @param gridSize フィールドの分割数（X方向とZ方向の両方に適用）
        /// @param amplitude 地形の高さスケール
        /// @param frequency ノイズ周波数（小さいほど緩やかな地形）
        /// @param octaves フラクタルノイズのオクターブ数
        /// @param seed ノイズシード値（地形バリエーション用）
        /// @return フィールド用MeshCreateDesc
        static MeshCreateDesc Field(float width = 100.0f, float depth = 100.0f, int gridSize = 10,
                                    float amplitude = 10.0f, float frequency = 0.05f, int octaves = 4, std::uint32_t seed = 0) {
            MeshCreateDesc d{}; d.m_type = MeshType::Field; d.m_width = width; d.m_depth = depth; d.m_fieldGridSize = gridSize;
            d.m_fieldAmplitude = amplitude; d.m_fieldFrequency = frequency; d.m_fieldOctaves = octaves; d.m_fieldSeed = seed;
            return d;
        }
    };


    /**
     * @brief MeshType ���� Mesh �𐶐�����t�@�N�g���[
     */
    class MeshFactory final {
    public:
        MeshFactory() = delete;

		/// @brief  ���b�V���𐶐�
		/// @param  device D3D11�f�o�C�X
		/// @param  outMesh �������ꂽMesh�̏o�͐�
        /// @param  type ���b�V���̎��
        /// @return �����Ȃ�true
        static bool Create(ID3D11Device* device, Mesh& outMesh, MeshType type);
		/// @brief  ���b�V���𐶐�
		/// @param  device D3D11�f�o�C�X
		/// @param  outMesh �������ꂽMesh�̏o�͐�
        /// @param  desc ���b�V���̐����p�����[�^
        /// @return �����Ȃ�true
        static bool Create(ID3D11Device* device, Mesh& outMesh, const MeshCreateDesc& desc);
    };

} // namespace Engine
