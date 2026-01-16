#pragma once

#include <d3d11.h>

#include "Engine/Graphics/Mesh.h"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"

namespace Engine {

    /**
     * @brief 標準頂点（DefaultVS に合わせる：Pos/Normal/Color/UV）
     */
    struct VertexPosNormColorUv final {
        Vector3 m_pos;
        Vector3 m_normal;
        Vector4 m_color;
        Vector2 m_uv;
    };

    class MeshPrimitives final {
    public:
        MeshPrimitives() = delete;

        //============================================================
        // Quad (XY平面, Z=0, 正面+Z)
        //============================================================
        static bool CreateQuad(
            ID3D11Device* device,
            Mesh& outMesh,
            float width = 1.0f,
            float height = 1.0f
        );

        //============================================================
        // Cube (中心原点, サイズ指定)
        //  - 各面ごとに頂点を分ける（法線/UVが綺麗）
        //============================================================
        static bool CreateCube(
            ID3D11Device* device,
            Mesh& outMesh,
            float sizeX = 1.0f,
            float sizeY = 1.0f,
            float sizeZ = 1.0f
        );

        //============================================================
        // Plane Grid (XZ平面, Y=0)
        //============================================================
        static bool CreatePlaneGrid(
            ID3D11Device* device,
            Mesh& outMesh,
            float width = 10.0f,
            float depth = 10.0f,
            int gridX = 10,   // 分割数（横）: 10 -> 11頂点
            int gridZ = 10    // 分割数（縦）
        );

        //============================================================
        // Sphere (UV sphere)
        //============================================================
        static bool CreateSphereUv(
            ID3D11Device* device,
            Mesh& outMesh,
            float radius = 0.5f,
            int slices = 32,
            int stacks = 16
        );

        //============================================================
        // Sphere (Icosphere)
        //  - subdivisions: 0=icosahedron, 1.. = subdivide
        //============================================================
        static bool CreateSphereIco(
            ID3D11Device* device,
            Mesh& outMesh,
            float radius = 0.5f,
            int subdivisions = 2
        );

        //============================================================
        // Capsule (Y軸方向)
        // height は「全長」（半球2つ込み）。height < 2*radius の場合は球に近づく
        //============================================================
        static bool CreateCapsule(
            ID3D11Device* device,
            Mesh& outMesh,
            float radius = 0.5f,
            float height = 2.0f,
            int slices = 32,
            int stacksHemisphere = 8,
            int stacksCylinder = 4
        );
    };

} // namespace Engine
