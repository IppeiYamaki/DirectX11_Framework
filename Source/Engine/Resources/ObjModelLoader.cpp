#include "ObjModelLoader.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "Engine/Core/Logger.h"

#include "Engine/Graphics/Model.h"
#include "Engine/Graphics/Shader.h"
#include "Engine/Resources/AssetManager.h"
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"

namespace Engine {

    namespace {

        struct VertexPosNormColorUv final {
            Vector3 m_pos;
            Vector3 m_normal;
            Vector4 m_color;
            Vector2 m_uv;
        };

        struct MtlInfo final {
            Vector3 m_kd{ 1,1,1 }; // diffuse
            Vector3 m_ka{ 0.2f,0.2f,0.2f }; // ambient
            Vector3 m_ks{ 0,0,0 }; // specular
            float   m_ns = 0.0f;   // shininess
            std::wstring m_mapKd{}; // texture
        };

        static std::string TrimLeft(std::string s) {
            size_t i = 0;
            while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n')) ++i;
            return s.substr(i);
        }

        static std::string Trim(std::string s) {
            s = TrimLeft(std::move(s));
            while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r' || s.back() == '\n')) s.pop_back();
            return s;
        }

        static bool StartsWith(const std::string& s, const char* prefix) {
            const size_t n = std::char_traits<char>::length(prefix);
            return s.size() >= n && s.compare(0, n, prefix) == 0;
        }

        // OBJ index: 1-based, negative = relative
        static int FixObjIndex(int idx, int count) {
            if (idx > 0) return idx - 1;
            if (idx < 0) return count + idx; // idx is negative
            return -1;
        }

        struct FaceIndex final {
            int v = -1;
            int vt = -1;
            int vn = -1;
        };

        static FaceIndex ParseFaceToken(const std::string& token) {
            // formats: v, v/vt, v//vn, v/vt/vn
            FaceIndex fi{};

            int parts[3] = { 0,0,0 };
            int partCount = 0;

            std::string cur;
            for (char c : token) {
                if (c == '/') {
                    if (partCount < 3) {
                        parts[partCount] = cur.empty() ? 0 : std::stoi(cur);
                        ++partCount;
                    }
                    cur.clear();
                }
                else {
                    cur.push_back(c);
                }
            }
            if (partCount < 3) {
                parts[partCount] = cur.empty() ? 0 : std::stoi(cur);
                ++partCount;
            }

            // parts: [v, vt, vn] (missing -> 0)
            fi.v = parts[0];
            if (partCount >= 2) fi.vt = parts[1];
            if (partCount >= 3) fi.vn = parts[2];

            return fi;
        }

        static bool LoadMtlFile(
            const std::filesystem::path& mtlPath,
            std::unordered_map<std::string, MtlInfo>& outMtls
        ) {
            std::ifstream file(mtlPath);
            if (!file.is_open()) {
                Logger::Error("LoadMtlFile failed: cannot open .mtl");
                return false;
            }

            std::string line;
            std::string currentName;
            MtlInfo current{};

            auto flush = [&]() {
                if (!currentName.empty()) {
                    outMtls[currentName] = current;
                }
                };

            while (std::getline(file, line)) {
                line = Trim(line);
                if (line.empty() || line[0] == '#') continue;

                std::istringstream iss(line);
                std::string key;
                iss >> key;

                if (key == "newmtl") {
                    flush();
                    iss >> currentName;
                    current = MtlInfo{};
                }
                else if (key == "Kd") {
                    iss >> current.m_kd.x >> current.m_kd.y >> current.m_kd.z;
                }
                else if (key == "Ka") {
                    iss >> current.m_ka.x >> current.m_ka.y >> current.m_ka.z;
                }
                else if (key == "Ks") {
                    iss >> current.m_ks.x >> current.m_ks.y >> current.m_ks.z;
                }
                else if (key == "Ns") {
                    iss >> current.m_ns;
                }
                else if (key == "map_Kd") {
                    // map_Kd の後ろはスペース入りパスがあるので残りを取る
                    std::string rest;
                    std::getline(iss, rest);
                    rest = Trim(rest);
                    if (!rest.empty()) {
                        std::filesystem::path tex = rest;
                        current.m_mapKd = tex.wstring();
                    }
                }
            }

            flush();
            return true;
        }

        static std::shared_ptr<Material> CreateMaterialFromMtl(
            ID3D11Device* device,
            AssetManager& assets,
            const std::filesystem::path& baseDir,
            const std::string& materialName,
            const MtlInfo* mtl
        ) {
            if (!device) return nullptr;

            auto mat = std::make_shared<Material>();
            if (!mat->Initialize(device)) {
                Logger::Error("CreateMaterialFromMtl failed: Material::Initialize failed.");
                return nullptr;
            }

            // Default shaders
            auto vs = assets.LoadVertexShader(L"Shaders/DefaultVS.cso");
            auto ps = assets.LoadPixelShader(L"Shaders/DefaultPS.cso");
            if (!vs || !ps) {
                Logger::Error("CreateMaterialFromMtl failed: load default shaders failed.");
                return nullptr;
            }

            auto il = assets.CreateInputLayout(
                L"DefaultPosNormColorUv",
                CreateDefaultPosNormColorUvLayout(),
                *vs
            );
            if (!il) {
                Logger::Error("CreateMaterialFromMtl failed: create input layout failed.");
                return nullptr;
            }

            mat->SetVertexShader(vs);
            mat->SetPixelShader(ps);
            mat->SetInputLayout(il);

            // params
            auto& p = mat->GetParams();
            if (mtl) {
                p.m_baseColor = Vector4(mtl->m_kd.x, mtl->m_kd.y, mtl->m_kd.z, 1.0f);
                p.m_ambient = Vector4(mtl->m_ka.x, mtl->m_ka.y, mtl->m_ka.z, 1.0f);
                p.m_specular = Vector4(mtl->m_ks.x, mtl->m_ks.y, mtl->m_ks.z, 1.0f);
                p.m_shininess = mtl->m_ns;

                // texture
                if (!mtl->m_mapKd.empty()) {
                    std::filesystem::path texPath = baseDir / std::filesystem::path(mtl->m_mapKd);
                    auto tex = assets.LoadTexture(texPath.wstring());
                    if (tex) {
                        mat->SetTexture(tex);
                        mat->EnableTexture(true);
                    }
                    else {
                        mat->EnableTexture(false);
                    }
                }
                else {
                    mat->EnableTexture(false);
                }
            }
            else {
                // fallback
                p.m_baseColor = Vector4(1, 1, 1, 1);
                p.m_ambient = Vector4(0.2f, 0.2f, 0.2f, 1);
                p.m_specular = Vector4(0, 0, 0, 1);
                p.m_shininess = 0.0f;
                mat->EnableTexture(false);
            }

            (void)materialName;
            return mat;
        }

        // key for dedup
        struct VertexKey final {
            int v = -1;
            int vt = -1;
            int vn = -1;

            bool operator==(const VertexKey& rhs) const noexcept {
                return v == rhs.v && vt == rhs.vt && vn == rhs.vn;
            }
        };

        struct VertexKeyHash final {
            size_t operator()(const VertexKey& k) const noexcept {
                // simple hash combine
                size_t h = 1469598103934665603ull;
                auto mix = [&](int x) {
                    h ^= static_cast<size_t>(x + 0x9e3779b9);
                    h *= 1099511628211ull;
                    };
                mix(k.v);
                mix(k.vt);
                mix(k.vn);
                return h;
            }
        };

    } // namespace

    std::shared_ptr<Model> LoadObjModel(ID3D11Device* device, AssetManager& assets, const std::wstring& objPath) {
        if (!device) {
            Logger::Error("LoadObjModel failed: device is null.");
            return nullptr;
        }

        const std::filesystem::path objFsPath(objPath);
        std::ifstream file(objFsPath);
        if (!file.is_open()) {
            Logger::Error("LoadObjModel failed: cannot open .obj");
            return nullptr;
        }

        const std::filesystem::path objDir = objFsPath.parent_path();

        std::vector<Vector3> positions;
        std::vector<Vector3> normals;
        std::vector<Vector2> uvs;

        std::vector<VertexPosNormColorUv> outVertices;
        std::vector<std::uint32_t> outIndices;

        std::unordered_map<std::string, MtlInfo> mtlInfos;
        std::unordered_map<std::string, std::shared_ptr<Material>> materialCache;

        auto getMaterial = [&](const std::string& name) -> std::shared_ptr<Material> {
            auto it = materialCache.find(name);
            if (it != materialCache.end()) return it->second;

            const MtlInfo* info = nullptr;
            auto itInfo = mtlInfos.find(name);
            if (itInfo != mtlInfos.end()) info = &itInfo->second;

            auto mat = CreateMaterialFromMtl(device, assets, objDir, name, info);
            materialCache[name] = mat;
            return mat;
            };

        // subset building (contiguous ranges)
        std::vector<ModelSubset> subsets;
        std::string currentMtlName = "Default";
        std::uint32_t currentStartIndex = 0;

        auto flushSubsetIfNeeded = [&](std::uint32_t endIndex) {
            const std::uint32_t count = (endIndex >= currentStartIndex) ? (endIndex - currentStartIndex) : 0;
            if (count == 0) return;

            ModelSubset s{};
            s.m_startIndex = currentStartIndex;
            s.m_indexCount = count;
            s.m_materialName = currentMtlName;
            s.m_material = getMaterial(currentMtlName);
            subsets.push_back(std::move(s));
            };

        // dedup map (only when normals exist in face)
        std::unordered_map<VertexKey, std::uint32_t, VertexKeyHash> dedup;

        std::string line;
        while (std::getline(file, line)) {
            line = Trim(line);
            if (line.empty() || line[0] == '#') continue;

            std::istringstream iss(line);
            std::string head;
            iss >> head;

            if (head == "v") {
                Vector3 p{};
                iss >> p.x >> p.y >> p.z;
                positions.push_back(p);
            }
            else if (head == "vn") {
                Vector3 n{};
                iss >> n.x >> n.y >> n.z;
                normals.push_back(n);
            }
            else if (head == "vt") {
                Vector2 t{};
                iss >> t.x >> t.y;
                // OBJはVが上下逆のことが多いので、必要ならここで (1 - v)
                // 今回はそのままにする（欲しければここを 1.0f - t.y に変更）
                uvs.push_back(t);
            }
            else if (head == "mtllib") {
                std::string mtlName;
                iss >> mtlName;
                if (!mtlName.empty()) {
                    std::filesystem::path mtlPath = objDir / std::filesystem::path(mtlName);
                    LoadMtlFile(mtlPath, mtlInfos);
                }
            }
            else if (head == "usemtl") {
                // 今までのsubsetを確定
                flushSubsetIfNeeded(static_cast<std::uint32_t>(outIndices.size()));
                currentStartIndex = static_cast<std::uint32_t>(outIndices.size());

                iss >> currentMtlName;
                if (currentMtlName.empty()) currentMtlName = "Default";
            }
            else if (head == "f") {
                // face tokens
                std::vector<FaceIndex> face;
                std::string tok;
                while (iss >> tok) {
                    face.push_back(ParseFaceToken(tok));
                }
                if (face.size() < 3) continue;

                // triangulate fan: (0,i,i+1)
                for (size_t i = 1; i + 1 < face.size(); ++i) {
                    FaceIndex a = face[0];
                    FaceIndex b = face[i];
                    FaceIndex c = face[i + 1];

                    // fix indices
                    a.v = FixObjIndex(a.v, static_cast<int>(positions.size()));
                    b.v = FixObjIndex(b.v, static_cast<int>(positions.size()));
                    c.v = FixObjIndex(c.v, static_cast<int>(positions.size()));

                    a.vt = FixObjIndex(a.vt, static_cast<int>(uvs.size()));
                    b.vt = FixObjIndex(b.vt, static_cast<int>(uvs.size()));
                    c.vt = FixObjIndex(c.vt, static_cast<int>(uvs.size()));

                    a.vn = FixObjIndex(a.vn, static_cast<int>(normals.size()));
                    b.vn = FixObjIndex(b.vn, static_cast<int>(normals.size()));
                    c.vn = FixObjIndex(c.vn, static_cast<int>(normals.size()));

                    const bool hasFaceNormals = (a.vn >= 0 && b.vn >= 0 && c.vn >= 0);

                    Vector3 faceNormal{ 0,1,0 };
                    if (!hasFaceNormals) {
                        // compute flat normal
                        const Vector3& p0 = positions[a.v];
                        const Vector3& p1 = positions[b.v];
                        const Vector3& p2 = positions[c.v];
                        const Vector3 e1 = p1 - p0;
                        const Vector3 e2 = p2 - p0;
                        faceNormal = Vector3::Cross(e1, e2).Normalized();
                    }

                    auto emitVertex = [&](const FaceIndex& fi) -> std::uint32_t {
                        if (fi.v < 0 || fi.v >= static_cast<int>(positions.size())) return 0;

                        if (hasFaceNormals) {
                            // dedup when normals exist
                            VertexKey key{ fi.v, fi.vt, fi.vn };
                            auto it = dedup.find(key);
                            if (it != dedup.end()) return it->second;

                            VertexPosNormColorUv v{};
                            v.m_pos = positions[fi.v];
                            v.m_normal = (fi.vn >= 0) ? normals[fi.vn] : Vector3(0, 1, 0);
                            v.m_uv = (fi.vt >= 0) ? uvs[fi.vt] : Vector2(0, 0);
                            v.m_color = Vector4(1, 1, 1, 1);

                            const std::uint32_t index = static_cast<std::uint32_t>(outVertices.size());
                            outVertices.push_back(v);
                            dedup[key] = index;
                            return index;
                        }
                        else {
                            // no normals: create unique vertex per face vertex (flat)
                            VertexPosNormColorUv v{};
                            v.m_pos = positions[fi.v];
                            v.m_normal = faceNormal;
                            v.m_uv = (fi.vt >= 0) ? uvs[fi.vt] : Vector2(0, 0);
                            v.m_color = Vector4(1, 1, 1, 1);

                            const std::uint32_t index = static_cast<std::uint32_t>(outVertices.size());
                            outVertices.push_back(v);
                            return index;
                        }
                        };

                    const std::uint32_t ia = emitVertex(a);
                    const std::uint32_t ib = emitVertex(b);
                    const std::uint32_t ic = emitVertex(c);

                    outIndices.push_back(ia);
                    outIndices.push_back(ib);
                    outIndices.push_back(ic);
                }
            }
        }

        // last subset
        flushSubsetIfNeeded(static_cast<std::uint32_t>(outIndices.size()));

        if (outVertices.empty() || outIndices.empty()) {
            Logger::Error("LoadObjModel failed: mesh data is empty.");
            return nullptr;
        }

        // ensure each subset has a material
        for (auto& s : subsets) {
            if (!s.m_material) {
                s.m_material = getMaterial(s.m_materialName);
            }
        }

        auto model = std::make_shared<Model>();
        if (!model->Initialize(
            device,
            outVertices.data(),
            static_cast<std::uint32_t>(sizeof(VertexPosNormColorUv)),
            static_cast<std::uint32_t>(outVertices.size()),
            outIndices.data(),
            static_cast<std::uint32_t>(outIndices.size()),
            std::move(subsets)
        )) {
            Logger::Error("LoadObjModel failed: Model::Initialize failed.");
            return nullptr;
        }

        return model;
    }

} // namespace Engine
