#include "MeshPrimitives.h"

#include <array>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace Engine {

    namespace {

        constexpr float kPi = 3.14159265358979323846f;

        inline float ClampFloat(float v, float a, float b) {
            return std::max(a, std::min(b, v));
        }

        inline int ClampInt(int v, int a, int b) {
            return std::max(a, std::min(b, v));
        }

        inline Vector3 NormalizeSafe(const Vector3& v) {
            const float lenSq = v.x * v.x + v.y * v.y + v.z * v.z;
            if (lenSq <= 1e-12f) return Vector3(0, 1, 0);
            const float invLen = 1.0f / std::sqrt(lenSq);
            return Vector3(v.x * invLen, v.y * invLen, v.z * invLen);
        }

        inline Vector2 SphericalUvFromNormal(const Vector3& n) {
            // n は正規化済み想定
            // u: 0..1 (atan2), v: 0..1 (asin)
            const float u = 0.5f + (std::atan2(n.z, n.x) / (2.0f * kPi));
            const float v = 0.5f - (std::asin(ClampFloat(n.y, -1.0f, 1.0f)) / kPi);
            return Vector2(u, v);
        }

    } // namespace

    //============================================================
    // Quad
    //============================================================
    bool MeshPrimitives::CreateQuad(ID3D11Device* device, Mesh& outMesh, float width, float height) {
        const float hw = width * 0.5f;
        const float hh = height * 0.5f;

        const Vector4 white(1, 1, 1, 1);

        const VertexPosNormColorUv v[] = {
            { Vector3(-hw, +hh, 0.0f), Vector3(0,0,1), white, Vector2(0,0) }, // LT
            { Vector3(+hw, +hh, 0.0f), Vector3(0,0,1), white, Vector2(1,0) }, // RT
            { Vector3(+hw, -hh, 0.0f), Vector3(0,0,1), white, Vector2(1,1) }, // RB
            { Vector3(-hw, -hh, 0.0f), Vector3(0,0,1), white, Vector2(0,1) }, // LB
        };

        const std::uint32_t i[] = { 0, 1, 2, 0, 2, 3 };
        return outMesh.Create(device, v, sizeof(VertexPosNormColorUv), 4, i, 6);
    }

    //============================================================
    // Cube
    //============================================================
    bool MeshPrimitives::CreateCube(ID3D11Device* device, Mesh& outMesh, float sizeX, float sizeY, float sizeZ) {
        const float hx = sizeX * 0.5f;
        const float hy = sizeY * 0.5f;
        const float hz = sizeZ * 0.5f;

        const Vector4 white(1, 1, 1, 1);

        std::array<VertexPosNormColorUv, 24> v{};

        // +Z
        v[0] = { Vector3(-hx, +hy, +hz), Vector3(0,0,1),  white, Vector2(0,0) };
        v[1] = { Vector3(+hx, +hy, +hz), Vector3(0,0,1),  white, Vector2(1,0) };
        v[2] = { Vector3(+hx, -hy, +hz), Vector3(0,0,1),  white, Vector2(1,1) };
        v[3] = { Vector3(-hx, -hy, +hz), Vector3(0,0,1),  white, Vector2(0,1) };

        // -Z
        v[4] = { Vector3(+hx, +hy, -hz), Vector3(0,0,-1), white, Vector2(0,0) };
        v[5] = { Vector3(-hx, +hy, -hz), Vector3(0,0,-1), white, Vector2(1,0) };
        v[6] = { Vector3(-hx, -hy, -hz), Vector3(0,0,-1), white, Vector2(1,1) };
        v[7] = { Vector3(+hx, -hy, -hz), Vector3(0,0,-1), white, Vector2(0,1) };

        // +X
        v[8] = { Vector3(+hx, +hy, +hz), Vector3(1,0,0),  white, Vector2(0,0) };
        v[9] = { Vector3(+hx, +hy, -hz), Vector3(1,0,0),  white, Vector2(1,0) };
        v[10] = { Vector3(+hx, -hy, -hz), Vector3(1,0,0),  white, Vector2(1,1) };
        v[11] = { Vector3(+hx, -hy, +hz), Vector3(1,0,0),  white, Vector2(0,1) };

        // -X
        v[12] = { Vector3(-hx, +hy, -hz), Vector3(-1,0,0), white, Vector2(0,0) };
        v[13] = { Vector3(-hx, +hy, +hz), Vector3(-1,0,0), white, Vector2(1,0) };
        v[14] = { Vector3(-hx, -hy, +hz), Vector3(-1,0,0), white, Vector2(1,1) };
        v[15] = { Vector3(-hx, -hy, -hz), Vector3(-1,0,0), white, Vector2(0,1) };

        // +Y
        v[16] = { Vector3(-hx, +hy, -hz), Vector3(0,1,0),  white, Vector2(0,0) };
        v[17] = { Vector3(+hx, +hy, -hz), Vector3(0,1,0),  white, Vector2(1,0) };
        v[18] = { Vector3(+hx, +hy, +hz), Vector3(0,1,0),  white, Vector2(1,1) };
        v[19] = { Vector3(-hx, +hy, +hz), Vector3(0,1,0),  white, Vector2(0,1) };

        // -Y
        v[20] = { Vector3(-hx, -hy, +hz), Vector3(0,-1,0), white, Vector2(0,0) };
        v[21] = { Vector3(+hx, -hy, +hz), Vector3(0,-1,0), white, Vector2(1,0) };
        v[22] = { Vector3(+hx, -hy, -hz), Vector3(0,-1,0), white, Vector2(1,1) };
        v[23] = { Vector3(-hx, -hy, -hz), Vector3(0,-1,0), white, Vector2(0,1) };

        std::array<std::uint32_t, 36> idx{};
        for (int face = 0; face < 6; ++face) {
            const std::uint32_t baseV = static_cast<std::uint32_t>(face * 4);
            const int baseI = face * 6;
            idx[baseI + 0] = baseV + 0;
            idx[baseI + 1] = baseV + 1;
            idx[baseI + 2] = baseV + 2;
            idx[baseI + 3] = baseV + 0;
            idx[baseI + 4] = baseV + 2;
            idx[baseI + 5] = baseV + 3;
        }

        return outMesh.Create(device, v.data(), sizeof(VertexPosNormColorUv), (std::uint32_t)v.size(), idx.data(), (std::uint32_t)idx.size());
    }

    //============================================================
    // Plane Grid (XZ)
    //============================================================
    bool MeshPrimitives::CreatePlaneGrid(ID3D11Device* device, Mesh& outMesh, float width, float depth, int gridX, int gridZ) {
        gridX = std::max(1, gridX);
        gridZ = std::max(1, gridZ);

        const int vertX = gridX + 1;
        const int vertZ = gridZ + 1;

        const float halfW = width * 0.5f;
        const float halfD = depth * 0.5f;

        const Vector4 white(1, 1, 1, 1);

        std::vector<VertexPosNormColorUv> vertices;
        vertices.reserve(static_cast<size_t>(vertX) * static_cast<size_t>(vertZ));

        for (int z = 0; z < vertZ; ++z) {
            const float tz = static_cast<float>(z) / static_cast<float>(gridZ);
            const float posZ = -halfD + tz * depth;

            for (int x = 0; x < vertX; ++x) {
                const float tx = static_cast<float>(x) / static_cast<float>(gridX);
                const float posX = -halfW + tx * width;

                VertexPosNormColorUv v{};
                v.m_pos = Vector3(posX, 0.0f, posZ);
                v.m_normal = Vector3(0, 1, 0);
                v.m_color = white;
                v.m_uv = Vector2(tx, 1.0f - tz);
                vertices.push_back(v);
            }
        }

        std::vector<std::uint32_t> indices;
        indices.reserve(static_cast<size_t>(gridX) * static_cast<size_t>(gridZ) * 6);

        for (int z = 0; z < gridZ; ++z) {
            for (int x = 0; x < gridX; ++x) {
                const std::uint32_t i0 = static_cast<std::uint32_t>(z * vertX + x);
                const std::uint32_t i1 = i0 + 1;
                const std::uint32_t i2 = i0 + static_cast<std::uint32_t>(vertX);
                const std::uint32_t i3 = i2 + 1;

                // (i0,i1,i3) (i0,i3,i2)
                indices.push_back(i0);
                indices.push_back(i1);
                indices.push_back(i3);

                indices.push_back(i0);
                indices.push_back(i3);
                indices.push_back(i2);
            }
        }

        return outMesh.Create(device, vertices.data(), sizeof(VertexPosNormColorUv),
            static_cast<std::uint32_t>(vertices.size()),
            indices.data(), static_cast<std::uint32_t>(indices.size()));
    }

    //============================================================
    // Sphere UV
    //============================================================
    bool MeshPrimitives::CreateSphereUv(ID3D11Device* device, Mesh& outMesh, float radius, int slices, int stacks) {
        slices = ClampInt(slices, 3, 256);
        stacks = ClampInt(stacks, 2, 256);
        radius = std::max(0.0001f, radius);

        const Vector4 white(1, 1, 1, 1);

        // (stacks+1) * (slices+1)
        std::vector<VertexPosNormColorUv> vertices;
        vertices.reserve(static_cast<size_t>(stacks + 1) * static_cast<size_t>(slices + 1));

        for (int stack = 0; stack <= stacks; ++stack) {
            const float v = static_cast<float>(stack) / static_cast<float>(stacks); // 0..1
            const float phi = (v * kPi); // 0..PI

            const float y = std::cos(phi);
            const float r = std::sin(phi);

            for (int slice = 0; slice <= slices; ++slice) {
                const float u = static_cast<float>(slice) / static_cast<float>(slices); // 0..1
                const float theta = u * (2.0f * kPi); // 0..2PI

                const float x = r * std::cos(theta);
                const float z = r * std::sin(theta);

                Vector3 n = NormalizeSafe(Vector3(x, y, z));
                Vector3 p = Vector3(n.x * radius, n.y * radius, n.z * radius);

                VertexPosNormColorUv vert{};
                vert.m_pos = p;
                vert.m_normal = n;
                vert.m_color = white;
                vert.m_uv = Vector2(u, 1.0f - v);
                vertices.push_back(vert);
            }
        }

        std::vector<std::uint32_t> indices;
        indices.reserve(static_cast<size_t>(stacks) * static_cast<size_t>(slices) * 6);

        const int ring = slices + 1;
        for (int stack = 0; stack < stacks; ++stack) {
            for (int slice = 0; slice < slices; ++slice) {
                const std::uint32_t i0 = static_cast<std::uint32_t>(stack * ring + slice);
                const std::uint32_t i1 = i0 + 1;
                const std::uint32_t i2 = i0 + static_cast<std::uint32_t>(ring);
                const std::uint32_t i3 = i2 + 1;

                // 2 triangles
                indices.push_back(i0);
                indices.push_back(i1);
                indices.push_back(i3);

                indices.push_back(i0);
                indices.push_back(i3);
                indices.push_back(i2);
            }
        }

        return outMesh.Create(device, vertices.data(), sizeof(VertexPosNormColorUv),
            static_cast<std::uint32_t>(vertices.size()),
            indices.data(), static_cast<std::uint32_t>(indices.size()));
    }

    //============================================================
    // Icosphere helper
    //============================================================
    namespace {

        struct Tri final { int a, b, c; };

        static int AddMidpoint(
            std::vector<Vector3>& verts,
            std::unordered_map<std::uint64_t, int>& cache,
            int i0, int i1
        ) {
            const int a = std::min(i0, i1);
            const int b = std::max(i0, i1);
            const std::uint64_t key = (static_cast<std::uint64_t>(static_cast<std::uint32_t>(a)) << 32) |
                static_cast<std::uint64_t>(static_cast<std::uint32_t>(b));

            auto it = cache.find(key);
            if (it != cache.end()) return it->second;

            const Vector3 v0 = verts[static_cast<size_t>(i0)];
            const Vector3 v1 = verts[static_cast<size_t>(i1)];
            const Vector3 mid = NormalizeSafe(Vector3(
                (v0.x + v1.x) * 0.5f,
                (v0.y + v1.y) * 0.5f,
                (v0.z + v1.z) * 0.5f
            ));

            const int idx = static_cast<int>(verts.size());
            verts.push_back(mid);
            cache.emplace(key, idx);
            return idx;
        }

    } // namespace

    //============================================================
    // Sphere Ico
    //============================================================
    bool MeshPrimitives::CreateSphereIco(ID3D11Device* device, Mesh& outMesh, float radius, int subdivisions) {
        radius = std::max(0.0001f, radius);
        subdivisions = ClampInt(subdivisions, 0, 6);

        const Vector4 white(1, 1, 1, 1);

        // Icosahedron
        const float t = (1.0f + std::sqrt(5.0f)) * 0.5f;

        std::vector<Vector3> baseVerts = {
            NormalizeSafe(Vector3(-1,  t,  0)),
            NormalizeSafe(Vector3(1,  t,  0)),
            NormalizeSafe(Vector3(-1, -t,  0)),
            NormalizeSafe(Vector3(1, -t,  0)),

            NormalizeSafe(Vector3(0, -1,  t)),
            NormalizeSafe(Vector3(0,  1,  t)),
            NormalizeSafe(Vector3(0, -1, -t)),
            NormalizeSafe(Vector3(0,  1, -t)),

            NormalizeSafe(Vector3(t,  0, -1)),
            NormalizeSafe(Vector3(t,  0,  1)),
            NormalizeSafe(Vector3(-t,  0, -1)),
            NormalizeSafe(Vector3(-t,  0,  1)),
        };

        std::vector<Tri> tris = {
            {0,11,5}, {0,5,1}, {0,1,7}, {0,7,10}, {0,10,11},
            {1,5,9}, {5,11,4}, {11,10,2}, {10,7,6}, {7,1,8},
            {3,9,4}, {3,4,2}, {3,2,6}, {3,6,8}, {3,8,9},
            {4,9,5}, {2,4,11}, {6,2,10}, {8,6,7}, {9,8,1}
        };

        // Subdivide
        for (int s = 0; s < subdivisions; ++s) {
            std::unordered_map<std::uint64_t, int> cache;
            cache.reserve(tris.size() * 3);

            std::vector<Tri> newTris;
            newTris.reserve(tris.size() * 4);

            for (const auto& tri : tris) {
                const int a = tri.a;
                const int b = tri.b;
                const int c = tri.c;

                const int ab = AddMidpoint(baseVerts, cache, a, b);
                const int bc = AddMidpoint(baseVerts, cache, b, c);
                const int ca = AddMidpoint(baseVerts, cache, c, a);

                newTris.push_back({ a,  ab, ca });
                newTris.push_back({ b,  bc, ab });
                newTris.push_back({ c,  ca, bc });
                newTris.push_back({ ab, bc, ca });
            }

            tris = std::move(newTris);
        }

        // Build vertices/indices (shared vertex)
        std::vector<VertexPosNormColorUv> vertices;
        vertices.reserve(baseVerts.size());

        for (const auto& n0 : baseVerts) {
            const Vector3 n = NormalizeSafe(n0);
            VertexPosNormColorUv v{};
            v.m_normal = n;
            v.m_pos = Vector3(n.x * radius, n.y * radius, n.z * radius);
            v.m_color = white;
            v.m_uv = SphericalUvFromNormal(n); // seamは出るけど最小実装としてOK
            vertices.push_back(v);
        }

        std::vector<std::uint32_t> indices;
        indices.reserve(tris.size() * 3);

        for (const auto& tri : tris) {
            indices.push_back(static_cast<std::uint32_t>(tri.a));
            indices.push_back(static_cast<std::uint32_t>(tri.b));
            indices.push_back(static_cast<std::uint32_t>(tri.c));
        }

        return outMesh.Create(device, vertices.data(), sizeof(VertexPosNormColorUv),
            static_cast<std::uint32_t>(vertices.size()),
            indices.data(), static_cast<std::uint32_t>(indices.size()));
    }

    //============================================================
    // Capsule
    //============================================================
    bool MeshPrimitives::CreateCapsule(
        ID3D11Device* device,
        Mesh& outMesh,
        float radius,
        float height,
        int slices,
        int stacksHemisphere,
        int stacksCylinder
    ) {
        radius = std::max(0.0001f, radius);
        height = std::max(radius * 2.0f, height);

        slices = ClampInt(slices, 3, 256);
        stacksHemisphere = ClampInt(stacksHemisphere, 2, 128);
        stacksCylinder = ClampInt(stacksCylinder, 1, 128);

        const Vector4 white(1, 1, 1, 1);

        const float cylH = std::max(0.0f, height - 2.0f * radius);
        const float halfCyl = cylH * 0.5f;

        // 総リング数（縦方向）
        // bottom hemi: stacksHemisphere
        // cylinder: stacksCylinder
        // top hemi: stacksHemisphere
        const int rings = stacksHemisphere + stacksCylinder + stacksHemisphere;
        const int vertsPerRing = slices + 1;

        const float minY = -halfCyl - radius;
        const float maxY = +halfCyl + radius;
        const float invH = 1.0f / (maxY - minY);

        std::vector<VertexPosNormColorUv> vertices;
        vertices.reserve(static_cast<size_t>(rings + 1) * static_cast<size_t>(vertsPerRing));

        // ring index 0..rings
        for (int r = 0; r <= rings; ++r) {
            float y = 0.0f;
            float ringRadius = radius;

            if (r < stacksHemisphere) {
                // bottom hemisphere: angle from -pi/2 .. 0
                const float t = static_cast<float>(r) / static_cast<float>(stacksHemisphere);
                const float angle = -kPi * 0.5f + t * (kPi * 0.5f);
                ringRadius = std::cos(angle) * radius;
                y = -halfCyl + std::sin(angle) * radius;
            }
            else if (r <= stacksHemisphere + stacksCylinder) {
                // cylinder: y from -halfCyl .. +halfCyl
                const float t = static_cast<float>(r - stacksHemisphere) / static_cast<float>(stacksCylinder);
                ringRadius = radius;
                y = -halfCyl + t * cylH;
            }
            else {
                // top hemisphere: angle from 0 .. +pi/2
                const float t = static_cast<float>(r - (stacksHemisphere + stacksCylinder)) / static_cast<float>(stacksHemisphere);
                const float angle = 0.0f + t * (kPi * 0.5f);
                ringRadius = std::cos(angle) * radius;
                y = +halfCyl + std::sin(angle) * radius;
            }

            const float v = 1.0f - ((y - minY) * invH);

            for (int s = 0; s <= slices; ++s) {
                const float u = static_cast<float>(s) / static_cast<float>(slices);
                const float theta = u * (2.0f * kPi);

                const float x = ringRadius * std::cos(theta);
                const float z = ringRadius * std::sin(theta);

                Vector3 normal{};
                if (r < stacksHemisphere) {
                    // bottom cap center at (0, -halfCyl, 0)
                    normal = NormalizeSafe(Vector3(x, y + halfCyl, z));
                }
                else if (r <= stacksHemisphere + stacksCylinder) {
                    // cylinder
                    normal = NormalizeSafe(Vector3(x, 0.0f, z));
                }
                else {
                    // top cap center at (0, +halfCyl, 0)
                    normal = NormalizeSafe(Vector3(x, y - halfCyl, z));
                }

                VertexPosNormColorUv vert{};
                vert.m_pos = Vector3(x, y, z);
                vert.m_normal = normal;
                vert.m_color = white;
                vert.m_uv = Vector2(u, v);

                vertices.push_back(vert);
            }
        }

        std::vector<std::uint32_t> indices;
        indices.reserve(static_cast<size_t>(rings) * static_cast<size_t>(slices) * 6);

        for (int r = 0; r < rings; ++r) {
            for (int s = 0; s < slices; ++s) {
                const std::uint32_t i0 = static_cast<std::uint32_t>(r * vertsPerRing + s);
                const std::uint32_t i1 = i0 + 1;
                const std::uint32_t i2 = i0 + static_cast<std::uint32_t>(vertsPerRing);
                const std::uint32_t i3 = i2 + 1;

                indices.push_back(i0);
                indices.push_back(i1);
                indices.push_back(i3);

                indices.push_back(i0);
                indices.push_back(i3);
                indices.push_back(i2);
            }
        }

        return outMesh.Create(device, vertices.data(), sizeof(VertexPosNormColorUv),
            static_cast<std::uint32_t>(vertices.size()),
            indices.data(), static_cast<std::uint32_t>(indices.size()));
    }

} // namespace Engine
