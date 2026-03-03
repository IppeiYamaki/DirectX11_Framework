#define NOMINMAX
#include "MeshPrimitives.h"

#include <array>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <cstdint>

#include "Engine/Math/PerlinNoise.h"

namespace Engine {

    namespace {
        constexpr float kPi = 3.14159265358979323846f;

        inline float ClampFloat(float v, float a, float b) { return (std::max)(a, (std::min)(b, v)); }
        inline int   ClampInt(int v, int a, int b) { return (std::max)(a, (std::min)(b, v)); }

        inline Vector3 NormalizeSafe(const Vector3& v) {
            const float lenSq = v.x * v.x + v.y * v.y + v.z * v.z;
            if (lenSq <= 1e-12f) return Vector3(0, 1, 0);
            const float invLen = 1.0f / std::sqrt(lenSq);
            return Vector3(v.x * invLen, v.y * invLen, v.z * invLen);
        }

        inline Vector2 SphericalUvFromNormal(const Vector3& n) {
            const float u = 0.5f + (std::atan2(n.z, n.x) / (2.0f * kPi));
            const float v = 0.5f - (std::asin(ClampFloat(n.y, -1.0f, 1.0f)) / kPi);
            return Vector2(u, v);
        }
    } // namespace

    bool MeshPrimitives::CreateQuad(ID3D11Device* device, Mesh& outMesh, float width, float height) {
        const float hw = width * 0.5f;
        const float hh = height * 0.5f;
        const Vector4 white(1, 1, 1, 1);

        const VertexPosNormColorUv v[] = {
            { Vector3(-hw, +hh, 0), Vector3(0,0,1), white, Vector2(0,0) },
            { Vector3(+hw, +hh, 0), Vector3(0,0,1), white, Vector2(1,0) },
            { Vector3(+hw, -hh, 0), Vector3(0,0,1), white, Vector2(1,1) },
            { Vector3(-hw, -hh, 0), Vector3(0,0,1), white, Vector2(0,1) },
        };
        const std::uint32_t i[] = { 0,1,2, 0,2,3 };
        return outMesh.Create(device, v, sizeof(VertexPosNormColorUv), 4, i, 6);
    }

    bool MeshPrimitives::CreateCube(ID3D11Device* device, Mesh& outMesh, float sx, float sy, float sz) {
        const float hx = sx * 0.5f, hy = sy * 0.5f, hz = sz * 0.5f;
        const Vector4 white(1, 1, 1, 1);

        std::array<VertexPosNormColorUv, 24> v{};

        // +Z
        v[0] = { Vector3(-hx,+hy,+hz), Vector3(0,0,1),  white, Vector2(0,0) };
        v[1] = { Vector3(+hx,+hy,+hz), Vector3(0,0,1),  white, Vector2(1,0) };
        v[2] = { Vector3(+hx,-hy,+hz), Vector3(0,0,1),  white, Vector2(1,1) };
        v[3] = { Vector3(-hx,-hy,+hz), Vector3(0,0,1),  white, Vector2(0,1) };
        // -Z
        v[4] = { Vector3(+hx,+hy,-hz), Vector3(0,0,-1), white, Vector2(0,0) };
        v[5] = { Vector3(-hx,+hy,-hz), Vector3(0,0,-1), white, Vector2(1,0) };
        v[6] = { Vector3(-hx,-hy,-hz), Vector3(0,0,-1), white, Vector2(1,1) };
        v[7] = { Vector3(+hx,-hy,-hz), Vector3(0,0,-1), white, Vector2(0,1) };
        // +X
        v[8] =  { Vector3(+hx,+hy,+hz), Vector3(1,0,0),  white, Vector2(0,0) };
        v[9] =  { Vector3(+hx,+hy,-hz), Vector3(1,0,0),  white, Vector2(1,0) };
        v[10] = { Vector3(+hx,-hy,-hz), Vector3(1,0,0),  white, Vector2(1,1) };
        v[11] = { Vector3(+hx,-hy,+hz), Vector3(1,0,0),  white, Vector2(0,1) };
        // -X
        v[12] = { Vector3(-hx,+hy,-hz), Vector3(-1,0,0), white, Vector2(0,0) };
        v[13] = { Vector3(-hx,+hy,+hz), Vector3(-1,0,0), white, Vector2(1,0) };
        v[14] = { Vector3(-hx,-hy,+hz), Vector3(-1,0,0), white, Vector2(1,1) };
        v[15] = { Vector3(-hx,-hy,-hz), Vector3(-1,0,0), white, Vector2(0,1) };
        // +Y
        v[16] = { Vector3(-hx,+hy,-hz), Vector3(0,1,0),  white, Vector2(0,0) };
        v[17] = { Vector3(+hx,+hy,-hz), Vector3(0,1,0),  white, Vector2(1,0) };
        v[18] = { Vector3(+hx,+hy,+hz), Vector3(0,1,0),  white, Vector2(1,1) };
        v[19] = { Vector3(-hx,+hy,+hz), Vector3(0,1,0),  white, Vector2(0,1) };
        // -Y
        v[20] = { Vector3(-hx,-hy,+hz), Vector3(0,-1,0), white, Vector2(0,0) };
        v[21] = { Vector3(+hx,-hy,+hz), Vector3(0,-1,0), white, Vector2(1,0) };
        v[22] = { Vector3(+hx,-hy,-hz), Vector3(0,-1,0), white, Vector2(1,1) };
        v[23] = { Vector3(-hx,-hy,-hz), Vector3(0,-1,0), white, Vector2(0,1) };

        std::array<std::uint32_t, 36> idx{};
        for (int face = 0; face < 6; ++face) {
            const std::uint32_t baseV = (std::uint32_t)(face * 4);
            const int baseI = face * 6;
            // Clockwise winding for outside-facing triangles (FrontCounterClockwise = FALSE)
            idx[baseI + 0] = baseV + 0; idx[baseI + 1] = baseV + 2; idx[baseI + 2] = baseV + 1;
            idx[baseI + 3] = baseV + 0; idx[baseI + 4] = baseV + 3; idx[baseI + 5] = baseV + 2;
        }
        return outMesh.Create(device, v.data(), sizeof(VertexPosNormColorUv), (std::uint32_t)v.size(),
            idx.data(), (std::uint32_t)idx.size());
    }

    bool MeshPrimitives::CreatePlaneGrid(ID3D11Device* device, Mesh& outMesh, float width, float depth, int gridX, int gridZ) {
        gridX = (std::max)(1, gridX);
        gridZ = (std::max)(1, gridZ);

        const int vx = gridX + 1;
        const int vz = gridZ + 1;

        const float halfW = width * 0.5f;
        const float halfD = depth * 0.5f;

        const Vector4 white(1, 1, 1, 1);

        std::vector<VertexPosNormColorUv> vertices;
        vertices.reserve((size_t)vx * (size_t)vz);

        for (int z = 0; z < vz; ++z) {
            const float tz = (float)z / (float)gridZ;
            const float posZ = -halfD + tz * depth;

            for (int x = 0; x < vx; ++x) {
                const float tx = (float)x / (float)gridX;
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
        indices.reserve((size_t)gridX * (size_t)gridZ * 6);

        for (int z = 0; z < gridZ; ++z) {
            for (int x = 0; x < gridX; ++x) {
                const std::uint32_t i0 = (std::uint32_t)(z * vx + x);
                const std::uint32_t i1 = i0 + 1;
                const std::uint32_t i2 = i0 + (std::uint32_t)vx;
                const std::uint32_t i3 = i2 + 1;

                indices.push_back(i0); indices.push_back(i1); indices.push_back(i3);
                indices.push_back(i0); indices.push_back(i3); indices.push_back(i2);
            }
        }

        return outMesh.Create(device, vertices.data(), sizeof(VertexPosNormColorUv),
            (std::uint32_t)vertices.size(),
            indices.data(), (std::uint32_t)indices.size());
    }

    bool MeshPrimitives::CreateSphereUv(ID3D11Device* device, Mesh& outMesh, float radius, int slices, int stacks) {
        slices = ClampInt(slices, 3, 256);
        stacks = ClampInt(stacks, 2, 256);
        radius = (std::max)(0.0001f, radius);

        const Vector4 white(1, 1, 1, 1);

        std::vector<VertexPosNormColorUv> vertices;
        vertices.reserve((size_t)(stacks + 1) * (size_t)(slices + 1));

        for (int st = 0; st <= stacks; ++st) {
            const float v = (float)st / (float)stacks;
            const float phi = v * kPi;

            const float y = std::cos(phi);
            const float r = std::sin(phi);

            for (int sl = 0; sl <= slices; ++sl) {
                const float u = (float)sl / (float)slices;
                const float theta = u * (2.0f * kPi);

                const float x = r * std::cos(theta);
                const float z = r * std::sin(theta);

                const Vector3 n = NormalizeSafe(Vector3(x, y, z));
                const Vector3 p = Vector3(n.x * radius, n.y * radius, n.z * radius);

                VertexPosNormColorUv vert{};
                vert.m_pos = p;
                vert.m_normal = n;
                vert.m_color = white;
                vert.m_uv = Vector2(u, 1.0f - v);
                vertices.push_back(vert);
            }
        }

        std::vector<std::uint32_t> indices;
        indices.reserve((size_t)stacks * (size_t)slices * 6);

        const int ring = slices + 1;
        for (int st = 0; st < stacks; ++st) {
            for (int sl = 0; sl < slices; ++sl) {
                const std::uint32_t i0 = (std::uint32_t)(st * ring + sl);
                const std::uint32_t i1 = i0 + 1;
                const std::uint32_t i2 = i0 + (std::uint32_t)ring;
                const std::uint32_t i3 = i2 + 1;

                indices.push_back(i0); indices.push_back(i1); indices.push_back(i3);
                indices.push_back(i0); indices.push_back(i3); indices.push_back(i2);
            }
        }

        return outMesh.Create(device, vertices.data(), sizeof(VertexPosNormColorUv),
            (std::uint32_t)vertices.size(),
            indices.data(), (std::uint32_t)indices.size());
    }

    // ---- icosphere helper ----
    namespace {
        struct Tri { int a, b, c; };

        static int AddMidpoint(std::vector<Vector3>& verts,
            std::unordered_map<std::uint64_t, int>& cache,
            int i0, int i1) {
            const int a = (std::min)(i0, i1);
            const int b = (std::max)(i0, i1);
            const std::uint64_t key = ((std::uint64_t)(std::uint32_t)a << 32) | (std::uint64_t)(std::uint32_t)b;

            auto it = cache.find(key);
            if (it != cache.end()) return it->second;

            const Vector3 v0 = verts[(size_t)i0];
            const Vector3 v1 = verts[(size_t)i1];
            const Vector3 mid = NormalizeSafe(Vector3((v0.x + v1.x) * 0.5f, (v0.y + v1.y) * 0.5f, (v0.z + v1.z) * 0.5f));

            const int idx = (int)verts.size();
            verts.push_back(mid);
            cache.emplace(key, idx);
            return idx;
        }
    } // namespace

    bool MeshPrimitives::CreateSphereIco(ID3D11Device* device, Mesh& outMesh, float radius, int subdivisions) {
        radius = (std::max)(0.0001f, radius);
        subdivisions = ClampInt(subdivisions, 0, 6);
        const Vector4 white(1, 1, 1, 1);

        const float t = (1.0f + std::sqrt(5.0f)) * 0.5f;

        std::vector<Vector3> baseVerts = {
            NormalizeSafe(Vector3(-1,  t,  0)), NormalizeSafe(Vector3(1,  t,  0)),
            NormalizeSafe(Vector3(-1, -t,  0)), NormalizeSafe(Vector3(1, -t,  0)),
            NormalizeSafe(Vector3(0, -1,  t)), NormalizeSafe(Vector3(0,  1,  t)),
            NormalizeSafe(Vector3(0, -1, -t)), NormalizeSafe(Vector3(0,  1, -t)),
            NormalizeSafe(Vector3(t,  0, -1)), NormalizeSafe(Vector3(t,  0,  1)),
            NormalizeSafe(Vector3(-t,  0, -1)), NormalizeSafe(Vector3(-t,  0,  1)),
        };

        std::vector<Tri> tris = {
            {0,11,5},{0,5,1},{0,1,7},{0,7,10},{0,10,11},
            {1,5,9},{5,11,4},{11,10,2},{10,7,6},{7,1,8},
            {3,9,4},{3,4,2},{3,2,6},{3,6,8},{3,8,9},
            {4,9,5},{2,4,11},{6,2,10},{8,6,7},{9,8,1}
        };

        for (int s = 0; s < subdivisions; ++s) {
            std::unordered_map<std::uint64_t, int> cache;
            cache.reserve(tris.size() * 3);

            std::vector<Tri> newTris;
            newTris.reserve(tris.size() * 4);

            for (const auto& tri : tris) {
                const int a = tri.a, b = tri.b, c = tri.c;
                const int ab = AddMidpoint(baseVerts, cache, a, b);
                const int bc = AddMidpoint(baseVerts, cache, b, c);
                const int ca = AddMidpoint(baseVerts, cache, c, a);

                newTris.push_back({ a,ab,ca });
                newTris.push_back({ b,bc,ab });
                newTris.push_back({ c,ca,bc });
                newTris.push_back({ ab,bc,ca });
            }
            tris = std::move(newTris);
        }

        std::vector<VertexPosNormColorUv> vertices;
        vertices.reserve(baseVerts.size());
        for (auto& n0 : baseVerts) {
            const Vector3 n = NormalizeSafe(n0);
            VertexPosNormColorUv v{};
            v.m_normal = n;
            v.m_pos = Vector3(n.x * radius, n.y * radius, n.z * radius);
            v.m_color = white;
            v.m_uv = SphericalUvFromNormal(n);
            vertices.push_back(v);
        }

        std::vector<std::uint32_t> indices;
        indices.reserve(tris.size() * 3);
        for (const auto& tri : tris) {
            indices.push_back((std::uint32_t)tri.a);
            indices.push_back((std::uint32_t)tri.b);
            indices.push_back((std::uint32_t)tri.c);
        }

        return outMesh.Create(device, vertices.data(), sizeof(VertexPosNormColorUv),
            (std::uint32_t)vertices.size(),
            indices.data(), (std::uint32_t)indices.size());
    }

    bool MeshPrimitives::CreateCapsule(ID3D11Device* device, Mesh& outMesh, float radius, float height,
        int slices, int stacksHemisphere, int stacksCylinder) {
        radius = (std::max)(0.0001f, radius);
        height = (std::max)(radius * 2.0f, height);

        slices = ClampInt(slices, 3, 256);
        stacksHemisphere = ClampInt(stacksHemisphere, 2, 128);
        stacksCylinder = ClampInt(stacksCylinder, 1, 128);

        const Vector4 white(1, 1, 1, 1);

        const float cylH = (std::max)(0.0f, height - 2.0f * radius);
        const float halfCyl = cylH * 0.5f;

        const int rings = stacksHemisphere + stacksCylinder + stacksHemisphere;
        const int vertsPerRing = slices + 1;

        const float minY = -halfCyl - radius;
        const float maxY = +halfCyl + radius;
        const float invH = 1.0f / (maxY - minY);

        std::vector<VertexPosNormColorUv> vertices;
        vertices.reserve((size_t)(rings + 1) * (size_t)vertsPerRing);

        for (int r = 0; r <= rings; ++r) {
            float y = 0.0f;
            float ringR = radius;

            if (r < stacksHemisphere) {
                const float t = (float)r / (float)stacksHemisphere;
                const float angle = -kPi * 0.5f + t * (kPi * 0.5f);
                ringR = std::cos(angle) * radius;
                y = -halfCyl + std::sin(angle) * radius;
            }
            else if (r <= stacksHemisphere + stacksCylinder) {
                const float t = (float)(r - stacksHemisphere) / (float)stacksCylinder;
                ringR = radius;
                y = -halfCyl + t * cylH;
            }
            else {
                const float t = (float)(r - (stacksHemisphere + stacksCylinder)) / (float)stacksHemisphere;
                const float angle = 0.0f + t * (kPi * 0.5f);
                ringR = std::cos(angle) * radius;
                y = +halfCyl + std::sin(angle) * radius;
            }

            const float v = 1.0f - ((y - minY) * invH);

            for (int s = 0; s <= slices; ++s) {
                const float u = (float)s / (float)slices;
                const float theta = u * (2.0f * kPi);

                const float x = ringR * std::cos(theta);
                const float z = ringR * std::sin(theta);

                Vector3 normal{};
                if (r < stacksHemisphere) normal = NormalizeSafe(Vector3(x, y + halfCyl, z));
                else if (r <= stacksHemisphere + stacksCylinder) normal = NormalizeSafe(Vector3(x, 0.0f, z));
                else normal = NormalizeSafe(Vector3(x, y - halfCyl, z));

                VertexPosNormColorUv vert{};
                vert.m_pos = Vector3(x, y, z);
                vert.m_normal = normal;
                vert.m_color = white;
                vert.m_uv = Vector2(u, v);
                vertices.push_back(vert);
            }
        }

        std::vector<std::uint32_t> indices;
        indices.reserve((size_t)rings * (size_t)slices * 6);

        for (int r = 0; r < rings; ++r) {
            for (int s = 0; s < slices; ++s) {
                const std::uint32_t i0 = (std::uint32_t)(r * vertsPerRing + s);
                const std::uint32_t i1 = i0 + 1;
                const std::uint32_t i2 = i0 + (std::uint32_t)vertsPerRing;
                const std::uint32_t i3 = i2 + 1;

                indices.push_back(i0); indices.push_back(i1); indices.push_back(i3);
                indices.push_back(i0); indices.push_back(i3); indices.push_back(i2);
            }
        }

        return outMesh.Create(device, vertices.data(), sizeof(VertexPosNormColorUv),
            (std::uint32_t)vertices.size(),
            indices.data(), (std::uint32_t)indices.size());
    }

    bool MeshPrimitives::CreateFieldGrid(ID3D11Device* device, Mesh& outMesh, float width, float depth, int gridSize,
                                          float amplitude, float frequency, int octaves, std::uint32_t seed) {
        gridSize = (std::max)(1, gridSize);
        octaves = (std::max)(1, (std::min)(8, octaves));  // Clamp octaves to [1, 8]

        const int vx = gridSize + 1;
        const int vz = gridSize + 1;

        const float halfW = width * 0.5f;
        const float halfD = depth * 0.5f;

        const Vector4 white(1, 1, 1, 1);

        // Initialize Perlin noise generator with seed
        PerlinNoise perlin(seed);

        // Generate vertex positions with Perlin noise heights
        std::vector<VertexPosNormColorUv> vertices;
        vertices.reserve((size_t)vx * (size_t)vz);

        for (int z = 0; z < vz; ++z) {
            const float tz = (float)z / (float)gridSize;
            const float posZ = -halfD + tz * depth;

            for (int x = 0; x < vx; ++x) {
                const float tx = (float)x / (float)gridSize;
                const float posX = -halfW + tx * width;

                // Generate height using fractal Perlin noise
                const double noiseValue = perlin.fractal(tx * width * frequency, tz * depth * frequency, octaves);
                
                // Map noise from [0, 1] to [-amplitude, +amplitude]
                const float height = static_cast<float>((noiseValue * 2.0 - 1.0) * amplitude);

                VertexPosNormColorUv v{};
                v.m_pos = Vector3(posX, height, posZ);
                v.m_normal = Vector3(0, 1, 0);  // Will recalculate later
                v.m_color = white;
                v.m_uv = Vector2(tx, 1.0f - tz);
                vertices.push_back(v);
            }
        }

        // Calculate proper normals based on adjacent vertices
        for (int z = 0; z < vz; ++z) {
            for (int x = 0; x < vx; ++x) {
                const int idx = z * vx + x;
                
                // Get neighboring vertices for normal calculation
                Vector3 left, right, up, down;
                
                if (x > 0) {
                    left = vertices[idx - 1].m_pos;
                } else {
                    // Mirror the adjacent vertex for boundary
                    const Vector3& current = vertices[idx].m_pos;
                    const Vector3& neighbor = vertices[idx + 1].m_pos;
                    left = Vector3(current.x - (neighbor.x - current.x), 
                                   current.y - (neighbor.y - current.y), 
                                   current.z);
                }
                
                if (x < vx - 1) {
                    right = vertices[idx + 1].m_pos;
                } else {
                    // Mirror the adjacent vertex for boundary
                    const Vector3& current = vertices[idx].m_pos;
                    const Vector3& neighbor = vertices[idx - 1].m_pos;
                    right = Vector3(current.x + (current.x - neighbor.x), 
                                    current.y + (current.y - neighbor.y), 
                                    current.z);
                }
                
                if (z > 0) {
                    up = vertices[idx - vx].m_pos;
                } else {
                    // Mirror the adjacent vertex for boundary
                    const Vector3& current = vertices[idx].m_pos;
                    const Vector3& neighbor = vertices[idx + vx].m_pos;
                    up = Vector3(current.x, 
                                 current.y - (neighbor.y - current.y), 
                                 current.z - (neighbor.z - current.z));
                }
                
                if (z < vz - 1) {
                    down = vertices[idx + vx].m_pos;
                } else {
                    // Mirror the adjacent vertex for boundary
                    const Vector3& current = vertices[idx].m_pos;
                    const Vector3& neighbor = vertices[idx - vx].m_pos;
                    down = Vector3(current.x, 
                                   current.y + (current.y - neighbor.y), 
                                   current.z + (current.z - neighbor.z));
                }
                
                // Calculate tangent vectors
                const Vector3 tangentX(right.x - left.x, right.y - left.y, right.z - left.z);
                const Vector3 tangentZ(down.x - up.x, down.y - up.y, down.z - up.z);
                
                // Cross product to get normal
                Vector3 normal(
                    tangentX.y * tangentZ.z - tangentX.z * tangentZ.y,
                    tangentX.z * tangentZ.x - tangentX.x * tangentZ.z,
                    tangentX.x * tangentZ.y - tangentX.y * tangentZ.x
                );
                
                // Normalize
                vertices[idx].m_normal = NormalizeSafe(normal);
            }
        }

        std::vector<std::uint32_t> indices;
        indices.reserve((size_t)gridSize * (size_t)gridSize * 6);

        for (int z = 0; z < gridSize; ++z) {
            for (int x = 0; x < gridSize; ++x) {
                const std::uint32_t i0 = (std::uint32_t)(z * vx + x);
                const std::uint32_t i1 = i0 + 1;
                const std::uint32_t i2 = i0 + (std::uint32_t)vx;
                const std::uint32_t i3 = i2 + 1;

                indices.push_back(i0);
                indices.push_back(i2);
                indices.push_back(i1);

                indices.push_back(i2);
                indices.push_back(i3);
                indices.push_back(i1);
            }
        }

        return outMesh.Create(device, vertices.data(), sizeof(VertexPosNormColorUv),
            (std::uint32_t)vertices.size(),
            indices.data(), (std::uint32_t)indices.size());
    }

} // namespace Engine
