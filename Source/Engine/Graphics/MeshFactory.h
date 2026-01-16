#pragma once

#include <d3d11.h>

#include "Engine/Graphics/Mesh.h"

namespace Engine {

    /**
     * @brief Unity風：プリミティブ種類
     */
    enum class MeshType {
        Quad,
        Cube,
        Plane,
        Sphere,
        Capsule,
    };

    /**
     * @brief 生成パラメータ（必要な時だけ使う）
     *
     * - Create(device, mesh, MeshType) は “デフォルト値” で作る
     * - 細かく変えたい場合に Create(device, mesh, desc) を使う
     */
    struct MeshCreateDesc final {
        MeshType m_type = MeshType::Quad;

        // Quad / Plane / Cube サイズ（CubeはXYZに使う）
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
    };


    /**
     * @brief MeshType から Mesh を生成するファクトリー
     */
    class MeshFactory final {
    public:
        MeshFactory() = delete;

        static bool Create(ID3D11Device* device, Mesh& outMesh, MeshType type);
        static bool Create(ID3D11Device* device, Mesh& outMesh, const MeshCreateDesc& desc);
    };

} // namespace Engine
