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

        // Quad / Plane
        float m_width = 1.0f;
        float m_height = 1.0f; // Quad 用
        float m_depth = 1.0f; // Plane 用

        int m_gridX = 10; // Plane grid
        int m_gridZ = 10;

        // Sphere
        enum class SphereKind { Uv, Ico };
        SphereKind m_sphereKind = SphereKind::Uv;

        float m_radius = 0.5f;
        int m_slices = 32;     // UV sphere / capsule
        int m_stacks = 16;     // UV sphere
        int m_subdivisions = 2;// Icosphere

        // Capsule
        float m_capsuleHeight = 2.0f;
        int m_stacksHemisphere = 8;
        int m_stacksCylinder = 4;

        // ---- “Unityっぽく”書けるように静的コンストラクタ ----
        static MeshCreateDesc Quad(float width = 1.0f, float height = 1.0f) {
            MeshCreateDesc d{};
            d.m_type = MeshType::Quad;
            d.m_width = width;
            d.m_height = height;
            return d;
        }

        static MeshCreateDesc Cube(float size = 1.0f) {
            MeshCreateDesc d{};
            d.m_type = MeshType::Cube;
            d.m_width = size;
            d.m_height = size;
            d.m_depth = size;
            return d;
        }

        static MeshCreateDesc Plane(float width = 10.0f, float depth = 10.0f, int gridX = 10, int gridZ = 10) {
            MeshCreateDesc d{};
            d.m_type = MeshType::Plane;
            d.m_width = width;
            d.m_depth = depth;
            d.m_gridX = gridX;
            d.m_gridZ = gridZ;
            return d;
        }

        static MeshCreateDesc SphereUv(float radius = 0.5f, int slices = 32, int stacks = 16) {
            MeshCreateDesc d{};
            d.m_type = MeshType::Sphere;
            d.m_sphereKind = SphereKind::Uv;
            d.m_radius = radius;
            d.m_slices = slices;
            d.m_stacks = stacks;
            return d;
        }

        static MeshCreateDesc SphereIco(float radius = 0.5f, int subdivisions = 2) {
            MeshCreateDesc d{};
            d.m_type = MeshType::Sphere;
            d.m_sphereKind = SphereKind::Ico;
            d.m_radius = radius;
            d.m_subdivisions = subdivisions;
            return d;
        }

        static MeshCreateDesc Capsule(float radius = 0.5f, float height = 2.0f, int slices = 32, int hemiStacks = 8, int cylStacks = 4) {
            MeshCreateDesc d{};
            d.m_type = MeshType::Capsule;
            d.m_radius = radius;
            d.m_capsuleHeight = height;
            d.m_slices = slices;
            d.m_stacksHemisphere = hemiStacks;
            d.m_stacksCylinder = cylStacks;
            return d;
        }
    };

    /**
     * @brief MeshType から Mesh を生成するファクトリー
     */
    class MeshFactory final {
    public:
        MeshFactory() = delete;

        // 種類だけ指定（デフォルトパラメータで生成）
        static bool Create(ID3D11Device* device, Mesh& outMesh, MeshType type);

        // パラメータ指定で生成
        static bool Create(ID3D11Device* device, Mesh& outMesh, const MeshCreateDesc& desc);
    };

} // namespace Engine
