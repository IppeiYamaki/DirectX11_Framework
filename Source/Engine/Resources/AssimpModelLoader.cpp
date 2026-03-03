#include "AssimpModelLoader.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <cwctype> 

#include <Windows.h>

// Assimp
#include <ThirdParty/Assimp/Include/assimp/Importer.hpp>
#include <ThirdParty/Assimp/Include/assimp/scene.h>
#include <ThirdParty/Assimp/Include/assimp/postprocess.h>
#include <ThirdParty/Assimp/Include/assimp/material.h>

#include "Engine/Core/Logger.h"
#include "Engine/Resources/AssetManager.h"
#include "Engine/Graphics/Model.h"
#include "Engine/Graphics/Shader.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Graphics/Material.h"
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

        static std::string WideToUtf8(const std::wstring& s) {
            if (s.empty()) return {};
            const int size = ::WideCharToMultiByte(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0, nullptr, nullptr);
            std::string out(size, '\0');
            ::WideCharToMultiByte(CP_UTF8, 0, s.c_str(), (int)s.size(), out.data(), size, nullptr, nullptr);
            return out;
        }

        static std::wstring Utf8ToWide(const std::string& s) {
            if (s.empty()) return {};
            const int size = ::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
            std::wstring out(size, L'\0');
            ::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), out.data(), size);
            return out;
        }

        static std::wstring GetFileExtensionLower(const std::wstring& filePath) {
            const std::filesystem::path p(filePath);
            std::wstring ext = p.extension().wstring();
            for (auto& c : ext) {
                c = static_cast<wchar_t>(std::towlower(c));
            }
            return ext;
        }

        static std::wstring GetRelativeTexturePath(
            const std::string& texturePath,
            const std::filesystem::path& modelDir
        ) {
            // FBX may store absolute paths - extract filename only
            std::filesystem::path texPath(Utf8ToWide(texturePath));
            std::wstring fileName = texPath.filename().wstring();
            
            if (fileName.empty()) {
                return Utf8ToWide(texturePath);
            }
            
            // Assume texture is in the same directory as the model
            std::filesystem::path fullPath = modelDir / fileName;
            return fullPath.wstring();
        }

        static std::shared_ptr<Material> CreateMaterialFromAssimp(
            ID3D11Device* device,
            AssetManager& assets,
            const std::filesystem::path& modelDir,
            const aiMaterial* aimat
        ) {
            if (!device) return nullptr;

            auto mat = std::make_shared<Material>();
            if (!mat->Initialize(device)) {
                Logger::Error("CreateMaterialFromAssimp failed: Material::Initialize failed.");
                return nullptr;
            }

            // Default shaders (same as ObjModelLoader)
            auto vs = assets.LoadVertexShader(L"Shaders/DefaultVS.cso");
            auto ps = assets.LoadPixelShader(L"Shaders/DefaultPS.cso");
            if (!vs || !ps) {
                Logger::Error("CreateMaterialFromAssimp failed: load default shaders failed.");
                return nullptr;
            }

            auto il = assets.CreateInputLayout(
                L"DefaultPosNormColorUv",
                CreateDefaultPosNormColorUvLayout(),
                *vs
            );
            if (!il) {
                Logger::Error("CreateMaterialFromAssimp failed: create input layout failed.");
                return nullptr;
            }

            mat->SetVertexShader(vs);
            mat->SetPixelShader(ps);
            mat->SetInputLayout(il);

            // Material parameters
            auto& p = mat->GetParams();

            aiColor3D kd(1, 1, 1);
            aiColor3D ka(0.2f, 0.2f, 0.2f);
            aiColor3D ks(0, 0, 0);
            float opacity = 1.0f;
            float shininess = 0.0f;

            if (aimat) {
                aimat->Get(AI_MATKEY_COLOR_DIFFUSE, kd);
                aimat->Get(AI_MATKEY_COLOR_AMBIENT, ka);
                aimat->Get(AI_MATKEY_COLOR_SPECULAR, ks);
                aimat->Get(AI_MATKEY_OPACITY, opacity);
                aimat->Get(AI_MATKEY_SHININESS, shininess);
            }

            p.m_baseColor = Vector4(kd.r, kd.g, kd.b, opacity);
            p.m_ambient = Vector4(ka.r, ka.g, ka.b, 1.0f);
            p.m_specular = Vector4(ks.r, ks.g, ks.b, 1.0f);
            p.m_shininess = shininess;

            // Texture
            bool hasTex = false;
            if (aimat) {
                aiString tex;
                if (aimat->GetTexture(aiTextureType_DIFFUSE, 0, &tex) == aiReturn_SUCCESS) {
                    if (tex.length > 0) {
                        // tex.C_Str() returns UTF-8 string, convert explicitly
                        const std::string texPathUtf8 = tex.C_Str();
                        const std::wstring texPath = GetRelativeTexturePath(texPathUtf8, modelDir);
                        auto t = assets.LoadTexture(texPath);
                        if (t) {
                            mat->SetTexture(t);
                            mat->EnableTexture(true);
                            hasTex = true;
                        }
                    }
                }
            }
            if (!hasTex) {
                mat->EnableTexture(false);
            }

            return mat;
        }

    } // namespace

    bool IsSupportedModelFormat(const std::wstring& filePath) {
        const std::wstring ext = GetFileExtensionLower(filePath);
        return (ext == L".obj" || ext == L".fbx" || ext == L".dae" ||
                ext == L".gltf" || ext == L".glb" || ext == L".blend");
    }

    std::shared_ptr<Model> LoadAssimpModel(
        ID3D11Device* device,
        AssetManager& assets,
        const std::wstring& modelPath
    ) {
        if (!device) {
            Logger::Error("LoadAssimpModel failed: device is null.");
            return nullptr;
        }

        // Note: modelPath is already resolved by AssetManager::LoadModel before calling this function.
        // Do not resolve the path again to avoid duplicating the base directory.
        const std::filesystem::path p(modelPath);
        const std::wstring pathWstring = p.wstring();

        // Check if file format is supported
        const std::wstring ext = GetFileExtensionLower(pathWstring);
        if (!IsSupportedModelFormat(pathWstring)) {
            Logger::Error("LoadAssimpModel failed: unsupported format - " + WideToUtf8(ext));
            return nullptr;
        }

        Assimp::Importer importer;

        // Import flags for static models (DirectX left-handed)
        const unsigned int flags =
            aiProcess_Triangulate |              // Triangulate polygons
            aiProcess_JoinIdenticalVertices |    // Join identical vertices
            aiProcess_GenSmoothNormals |         // Generate smooth normals
            aiProcess_CalcTangentSpace |         // Calculate tangent space
            aiProcess_ImproveCacheLocality |     // Improve cache locality
            aiProcess_ConvertToLeftHanded |      // Convert to left-handed (DirectX)
            aiProcess_GlobalScale;               // Apply global scale (for FBX cm->m)

        const std::string pathUtf8 = WideToUtf8(pathWstring);
        const aiScene* scene = importer.ReadFile(pathUtf8, flags);

        if (!scene || !scene->mRootNode) {
            Logger::Error("LoadAssimpModel failed: " + std::string(importer.GetErrorString()));
            return nullptr;
        }

        const std::filesystem::path modelDir = p.parent_path();

        // Build materials
        std::vector<std::shared_ptr<Material>> materialByIndex;
        materialByIndex.resize(scene->mNumMaterials);

        auto getMaterial = [&](unsigned int matIndex) -> std::shared_ptr<Material> {
            if (matIndex >= materialByIndex.size()) return nullptr;
            if (materialByIndex[matIndex]) return materialByIndex[matIndex];

            const aiMaterial* aim = scene->mMaterials[matIndex];
            materialByIndex[matIndex] = CreateMaterialFromAssimp(device, assets, modelDir, aim);
            return materialByIndex[matIndex];
        };

        // Collect all vertices and indices from all meshes
        std::vector<VertexPosNormColorUv> vertices;
        std::vector<std::uint32_t> indices;
        std::vector<ModelSubset> subsets;

        for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
            const aiMesh* mesh = scene->mMeshes[m];
            if (!mesh) continue;

            const std::uint32_t baseVertex = static_cast<std::uint32_t>(vertices.size());
            const std::uint32_t startIndex = static_cast<std::uint32_t>(indices.size());

            // Add vertices
            for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
                VertexPosNormColorUv vert{};
                
                // Position
                vert.m_pos = Vector3(
                    mesh->mVertices[v].x,
                    mesh->mVertices[v].y,
                    mesh->mVertices[v].z
                );

                // Normal
                if (mesh->HasNormals()) {
                    vert.m_normal = Vector3(
                        mesh->mNormals[v].x,
                        mesh->mNormals[v].y,
                        mesh->mNormals[v].z
                    );
                }
                else {
                    vert.m_normal = Vector3(0.0f, 1.0f, 0.0f);
                }

                // UV coordinates
                if (mesh->HasTextureCoords(0)) {
                    vert.m_uv = Vector2(
                        mesh->mTextureCoords[0][v].x,
                        mesh->mTextureCoords[0][v].y
                    );
                }
                else {
                    vert.m_uv = Vector2(0.0f, 0.0f);
                }

                // Vertex color
                if (mesh->HasVertexColors(0)) {
                    vert.m_color = Vector4(
                        mesh->mColors[0][v].r,
                        mesh->mColors[0][v].g,
                        mesh->mColors[0][v].b,
                        mesh->mColors[0][v].a
                    );
                }
                else {
                    vert.m_color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
                }

                vertices.push_back(vert);
            }

            // Add indices (triangulated faces)
            for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
                const aiFace& face = mesh->mFaces[f];
                if (face.mNumIndices != 3) {
                    // Should be triangulated by aiProcess_Triangulate
                    Logger::Warn("LoadAssimpModel: Skipping non-triangulated face with " + 
                                 std::to_string(face.mNumIndices) + " indices");
                    continue;
                }

                indices.push_back(baseVertex + face.mIndices[0]);
                indices.push_back(baseVertex + face.mIndices[1]);
                indices.push_back(baseVertex + face.mIndices[2]);
            }

            const std::uint32_t endIndex = static_cast<std::uint32_t>(indices.size());
            const std::uint32_t indexCount = endIndex - startIndex;

            if (indexCount > 0) {
                // Create subset for this mesh
                ModelSubset subset{};
                subset.m_startIndex = startIndex;
                subset.m_indexCount = indexCount;
                subset.m_materialName = mesh->mName.C_Str();
                subset.m_material = getMaterial(mesh->mMaterialIndex);
                subsets.push_back(std::move(subset));
            }
        }

        if (vertices.empty() || indices.empty()) {
            Logger::Error("LoadAssimpModel failed: mesh data is empty.");
            return nullptr;
        }

        // Create Model object
        auto model = std::make_shared<Model>();
        if (!model->Initialize(
            device,
            vertices.data(),
            static_cast<std::uint32_t>(sizeof(VertexPosNormColorUv)),
            static_cast<std::uint32_t>(vertices.size()),
            indices.data(),
            static_cast<std::uint32_t>(indices.size()),
            std::move(subsets)
        )) {
            Logger::Error("LoadAssimpModel failed: Model::Initialize failed.");
            return nullptr;
        }

        Logger::Info("Model loaded successfully: " + pathUtf8 + 
                     " (Meshes: " + std::to_string(scene->mNumMeshes) + 
                     ", Materials: " + std::to_string(scene->mNumMaterials) + ")");

        return model;
    }

} // namespace Engine
