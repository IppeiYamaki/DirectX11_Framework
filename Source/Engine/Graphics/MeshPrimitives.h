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

        static bool CreateQuad(ID3D11Device* device, Mesh& outMesh, float width = 1.0f, float height = 1.0f);
        static bool CreateCube(ID3D11Device* device, Mesh& outMesh, float sx = 1.0f, float sy = 1.0f, float sz = 1.0f);
        static bool CreatePlaneGrid(ID3D11Device* device, Mesh& outMesh, float width = 10.0f, float depth = 10.0f, int gridX = 10, int gridZ = 10);

        static bool CreateSphereUv(ID3D11Device* device, Mesh& outMesh, float radius = 0.5f, int slices = 32, int stacks = 16);
        static bool CreateSphereIco(ID3D11Device* device, Mesh& outMesh, float radius = 0.5f, int subdivisions = 2);

        static bool CreateCapsule(ID3D11Device* device, Mesh& outMesh, float radius = 0.5f, float height = 2.0f, int slices = 32, int stacksHemisphere = 8, int stacksCylinder = 4);
    };

} // namespace Engine
