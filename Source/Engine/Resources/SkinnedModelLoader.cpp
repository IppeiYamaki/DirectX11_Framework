#include "SkinnedModelLoader.h"

#include <filesystem>
#include <unordered_map>
#include <vector>

#include <Windows.h>

// Assimp
#include <ThirdParty/Assimp/Include/assimp/Importer.hpp>
#include <ThirdParty/Assimp/Include/assimp/scene.h>
#include <ThirdParty/Assimp/Include/assimp/postprocess.h>
#include <ThirdParty/Assimp/Include/assimp/material.h>

#include "Engine/Core/Logger.h"
#include "Engine/Resources/AssetManager.h"
#include "Engine/Graphics/SkinnedModel.h"
#include "Engine/Graphics/Shader.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Graphics/Material.h"

namespace Engine {

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

    static DirectX::XMFLOAT4X4 ToXMFLOAT4X4(const aiMatrix4x4& m) {
        DirectX::XMFLOAT4X4 o{};
        o._11 = m.a1; o._12 = m.a2; o._13 = m.a3; o._14 = m.a4;
        o._21 = m.b1; o._22 = m.b2; o._23 = m.b3; o._24 = m.b4;
        o._31 = m.c1; o._32 = m.c2; o._33 = m.c3; o._34 = m.c4;
        o._41 = m.d1; o._42 = m.d2; o._43 = m.d3; o._44 = m.d4;
        return o;
    }

    static std::shared_ptr<Material> CreateSkinnedMaterial(
        ID3D11Device* device,
        AssetManager& assets,
        const std::filesystem::path& modelDir,
        const aiMaterial* aimat
    ) {
        auto mat = std::make_shared<Material>();
        if (!mat->Initialize(device)) return nullptr;

        auto vs = assets.LoadVertexShader(L"Shaders/SkinnedVS.cso");
        auto ps = assets.LoadPixelShader(L"Shaders/DefaultPS.cso");
        if (!vs || !ps) {
            Logger::Error("CreateSkinnedMaterial failed: missing shaders.");
            return nullptr;
        }

        auto il = assets.CreateInputLayout(L"SkinnedPosNormColorUvWeights", CreateSkinnedPosNormColorUvWeightsLayout(), *vs);
        if (!il) return nullptr;

        mat->SetVertexShader(vs);
        mat->SetPixelShader(ps);
        mat->SetInputLayout(il);

        aiColor3D kd(1, 1, 1);
        float opacity = 1.0f;
        if (aimat) {
            aimat->Get(AI_MATKEY_COLOR_DIFFUSE, kd);
            aimat->Get(AI_MATKEY_OPACITY, opacity);
        }

        auto& p = mat->GetParams();
        p.m_baseColor = Vector4(kd.r, kd.g, kd.b, opacity);
        p.m_ambient = Vector4(0.2f, 0.2f, 0.2f, 1.0f);

        bool hasTex = false;
        if (aimat) {
            aiString tex;
            if (aimat->GetTexture(aiTextureType_DIFFUSE, 0, &tex) == aiReturn_SUCCESS) {
                if (tex.length > 0) {
                    const std::filesystem::path texPath = modelDir / std::filesystem::path(Utf8ToWide(tex.C_Str()));
                    auto t = assets.LoadTexture(texPath.wstring());
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

    struct NodeBuildResult {
        int m_rootIndex = 0;
        std::vector<SkinnedNode> m_nodes;
        std::unordered_map<std::string, int> m_nameToIndex;
    };

    static void BuildNodesRecursive(
        const aiNode* node,
        int parentIndex,
        NodeBuildResult& out
    ) {
        const int myIndex = (int)out.m_nodes.size();
        out.m_nodes.push_back(SkinnedNode{});
        out.m_nodes.back().m_name = node->mName.C_Str();
        out.m_nodes.back().m_parent = parentIndex;
        out.m_nodes.back().m_defaultLocal = ToXMFLOAT4X4(node->mTransformation);

        out.m_nameToIndex[out.m_nodes.back().m_name] = myIndex;

        if (parentIndex >= 0) {
            out.m_nodes[parentIndex].m_children.push_back(myIndex);
        }

        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            BuildNodesRecursive(node->mChildren[i], myIndex, out);
        }
    }

    static void AddInfluence(SkinnedVertex& v, std::uint32_t boneIndex, float weight) {
        int slot = -1;
        for (int i = 0; i < 4; ++i) {
            if (v.m_boneWeights[i] == 0.0f) { slot = i; break; }
        }
        if (slot < 0) {
            int minI = 0;
            for (int i = 1; i < 4; ++i) {
                if (v.m_boneWeights[i] < v.m_boneWeights[minI]) minI = i;
            }
            if (weight <= v.m_boneWeights[minI]) return;
            slot = minI;
        }

        v.m_boneIndices[slot] = boneIndex;
        v.m_boneWeights[slot] = weight;
    }

    std::shared_ptr<SkinnedModel> LoadSkinnedModel(
        ID3D11Device* device,
        AssetManager& assets,
        const std::wstring& modelPath
    ) {
        if (!device) {
            Logger::Error("LoadSkinnedModel failed: device is null.");
            return nullptr;
        }

        std::filesystem::path p(modelPath);
        if (!p.is_absolute()) {
            p = std::filesystem::path(assets.GetBaseDirectory()) / p;
        }

        Assimp::Importer importer;

        const unsigned int flags =
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_GenSmoothNormals |
            aiProcess_LimitBoneWeights |
            aiProcess_ImproveCacheLocality |
            aiProcess_ConvertToLeftHanded;

        const std::string pathUtf8 = WideToUtf8(p.wstring());
        const aiScene* scene = importer.ReadFile(pathUtf8, flags);

        if (!scene || !scene->mRootNode) {
            Logger::Error("LoadSkinnedModel failed: importer.ReadFile failed.");
            return nullptr;
        }

        NodeBuildResult nb{};
        BuildNodesRecursive(scene->mRootNode, -1, nb);
        nb.m_rootIndex = 0;

        std::unordered_map<std::string, std::uint32_t> boneNameToIndex;
        std::vector<SkinnedBone> bones;

        std::vector<SkinnedVertex> vertices;
        std::vector<std::uint32_t> indices;
        std::vector<SkinnedSubset> subsets;

        const std::filesystem::path modelDir = p.parent_path();

        std::vector<std::shared_ptr<Material>> materialByIndex;
        materialByIndex.resize(scene->mNumMaterials);

        auto getMaterial = [&](unsigned int matIndex) -> std::shared_ptr<Material> {
            if (matIndex >= materialByIndex.size()) return nullptr;
            if (materialByIndex[matIndex]) return materialByIndex[matIndex];

            const aiMaterial* aim = scene->mMaterials[matIndex];
            materialByIndex[matIndex] = CreateSkinnedMaterial(device, assets, modelDir, aim);
            return materialByIndex[matIndex];
            };

        for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
            const aiMesh* mesh = scene->mMeshes[m];
            if (!mesh) continue;

            const std::uint32_t baseVertex = (std::uint32_t)vertices.size();
            const std::uint32_t startIndex = (std::uint32_t)indices.size();

            vertices.resize(vertices.size() + mesh->mNumVertices);
            for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
                SkinnedVertex sv{};
                sv.m_position = { mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z };

                if (mesh->HasNormals()) {
                    sv.m_normal = { mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z };
                }
                else {
                    sv.m_normal = { 0,1,0 };
                }

                if (mesh->HasTextureCoords(0)) {
                    sv.m_uv = { mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y };
                }

                vertices[baseVertex + v] = sv;
            }

            indices.reserve(indices.size() + mesh->mNumFaces * 3);
            for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
                const aiFace& face = mesh->mFaces[f];
                if (face.mNumIndices != 3) continue;

                indices.push_back(baseVertex + face.mIndices[0]);
                indices.push_back(baseVertex + face.mIndices[1]);
                indices.push_back(baseVertex + face.mIndices[2]);
            }

            const std::uint32_t endIndex = (std::uint32_t)indices.size();
            const std::uint32_t indexCount = endIndex - startIndex;

            for (unsigned int b = 0; b < mesh->mNumBones; ++b) {
                const aiBone* ab = mesh->mBones[b];
                if (!ab) continue;

                const std::string boneName = ab->mName.C_Str();

                std::uint32_t boneIndex = 0;
                auto it = boneNameToIndex.find(boneName);
                if (it == boneNameToIndex.end()) {
                    boneIndex = (std::uint32_t)bones.size();
                    boneNameToIndex[boneName] = boneIndex;

                    SkinnedBone bone{};
                    bone.m_name = boneName;

                    auto itNode = nb.m_nameToIndex.find(boneName);
                    bone.m_nodeIndex = (itNode != nb.m_nameToIndex.end()) ? itNode->second : -1;
                    bone.m_offset = ToXMFLOAT4X4(ab->mOffsetMatrix);
                    bones.push_back(bone);
                }
                else {
                    boneIndex = it->second;
                }

                for (unsigned int w = 0; w < ab->mNumWeights; ++w) {
                    const aiVertexWeight& vw = ab->mWeights[w];
                    const std::uint32_t vid = baseVertex + vw.mVertexId;
                    if (vid >= vertices.size()) continue;
                    AddInfluence(vertices[vid], boneIndex, vw.mWeight);
                }
            }

            for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
                SkinnedVertex& sv = vertices[baseVertex + v];
                float sum = sv.m_boneWeights[0] + sv.m_boneWeights[1] + sv.m_boneWeights[2] + sv.m_boneWeights[3];
                if (sum > 0.0f) {
                    const float inv = 1.0f / sum;
                    for (int i = 0; i < 4; ++i) sv.m_boneWeights[i] *= inv;
                }
                else {
                    sv.m_boneIndices[0] = 0;
                    sv.m_boneWeights[0] = 1.0f;
                }
            }

            SkinnedSubset subset{};
            subset.m_startIndex = startIndex;
            subset.m_indexCount = indexCount;
            subset.m_material = getMaterial(mesh->mMaterialIndex);
            subsets.push_back(std::move(subset));
        }

        if (bones.empty()) {
            Logger::Error("LoadSkinnedModel failed: no bones in mesh.");
            return nullptr;
        }

        std::unordered_map<std::string, SkinnedClip> clips;

        if (scene->HasAnimations()) {
            for (unsigned int a = 0; a < scene->mNumAnimations; ++a) {
                const aiAnimation* anim = scene->mAnimations[a];
                if (!anim) continue;

                std::string clipName = anim->mName.length > 0 ? anim->mName.C_Str() : ("Anim" + std::to_string(a));

                SkinnedClip clip{};
                for (unsigned int c = 0; c < anim->mNumChannels; ++c) {
                    const aiNodeAnim* ch = anim->mChannels[c];
                    if (!ch) continue;

                    const std::string nodeName = ch->mNodeName.C_Str();
                    auto itNode = nb.m_nameToIndex.find(nodeName);
                    if (itNode == nb.m_nameToIndex.end()) continue;

                    SkinnedChannel sc{};

                    sc.m_positions.reserve(ch->mNumPositionKeys);
                    for (unsigned int i = 0; i < ch->mNumPositionKeys; ++i) {
                        const aiVector3D& v = ch->mPositionKeys[i].mValue;
                        sc.m_positions.push_back({ v.x, v.y, v.z });
                    }

                    sc.m_rotations.reserve(ch->mNumRotationKeys);
                    for (unsigned int i = 0; i < ch->mNumRotationKeys; ++i) {
                        const aiQuaternion& q = ch->mRotationKeys[i].mValue;
                        sc.m_rotations.push_back({ q.x, q.y, q.z, q.w });
                    }

                    sc.m_scales.reserve(ch->mNumScalingKeys);
                    for (unsigned int i = 0; i < ch->mNumScalingKeys; ++i) {
                        const aiVector3D& s = ch->mScalingKeys[i].mValue;
                        sc.m_scales.push_back({ s.x, s.y, s.z });
                    }

                    clip.m_channels[itNode->second] = std::move(sc);
                }

                clips[clipName] = std::move(clip);
            }
        }
        else {
            Logger::Info("LoadSkinnedModel: scene has no animations. (mesh only)");
        }

        auto model = std::make_shared<SkinnedModel>();
        if (!model->Initialize(
            device,
            vertices,
            indices,
            std::move(subsets),
            std::move(nb.m_nodes),
            std::move(bones),
            std::move(clips),
            nb.m_rootIndex
        )) {
            Logger::Error("LoadSkinnedModel failed: SkinnedModel::Initialize failed.");
            return nullptr;
        }

        return model;
    }

    // ★別ファイルからアニメを追加する
    bool AddAnimationFromFile(
        SkinnedModel& model,
        AssetManager& assets,
        const std::wstring& animPath,
        const std::string& clipName
    ) {
        std::filesystem::path p(animPath);
        if (!p.is_absolute()) {
            p = std::filesystem::path(assets.GetBaseDirectory()) / p;
        }

        Assimp::Importer importer;

        const unsigned int flags =
            aiProcess_ConvertToLeftHanded;

        const std::string pathUtf8 = WideToUtf8(p.wstring());
        const aiScene* scene = importer.ReadFile(pathUtf8, flags);

        if (!scene) {
            Logger::Error("AddAnimationFromFile failed: importer.ReadFile failed.");
            return false;
        }
        if (!scene->HasAnimations()) {
            Logger::Error("AddAnimationFromFile failed: file has no animations.");
            return false;
        }

        bool anyAdded = false;

        for (unsigned int a = 0; a < scene->mNumAnimations; ++a) {
            const aiAnimation* anim = scene->mAnimations[a];
            if (!anim) continue;

            // 名前決定：引数優先、空ならファイル内名、さらに空ならAnimN
            std::string outName = clipName;
            if (outName.empty()) {
                outName = (anim->mName.length > 0) ? anim->mName.C_Str() : ("Anim" + std::to_string(a));
            }
            else {
                // ファイルに複数アニメがある場合は被らないように suffix
                if (scene->mNumAnimations > 1) {
                    std::string suffix = (anim->mName.length > 0) ? anim->mName.C_Str() : std::to_string(a);
                    outName = outName + "_" + suffix;
                }
            }

            SkinnedClip clip{};

            for (unsigned int c = 0; c < anim->mNumChannels; ++c) {
                const aiNodeAnim* ch = anim->mChannels[c];
                if (!ch) continue;

                const std::string nodeName = ch->mNodeName.C_Str();
                const int nodeIndex = model.FindNodeIndexByName(nodeName);
                if (nodeIndex < 0) {
                    // 骨名/ノード名が一致しないものは無視（リターゲットしない方針）
                    continue;
                }

                SkinnedChannel sc{};

                sc.m_positions.reserve(ch->mNumPositionKeys);
                for (unsigned int i = 0; i < ch->mNumPositionKeys; ++i) {
                    const aiVector3D& v = ch->mPositionKeys[i].mValue;
                    sc.m_positions.push_back({ v.x, v.y, v.z });
                }

                sc.m_rotations.reserve(ch->mNumRotationKeys);
                for (unsigned int i = 0; i < ch->mNumRotationKeys; ++i) {
                    const aiQuaternion& q = ch->mRotationKeys[i].mValue;
                    sc.m_rotations.push_back({ q.x, q.y, q.z, q.w });
                }

                sc.m_scales.reserve(ch->mNumScalingKeys);
                for (unsigned int i = 0; i < ch->mNumScalingKeys; ++i) {
                    const aiVector3D& s = ch->mScalingKeys[i].mValue;
                    sc.m_scales.push_back({ s.x, s.y, s.z });
                }

                clip.m_channels[nodeIndex] = std::move(sc);
            }

            if (!clip.m_channels.empty()) {
                model.AddClip(outName, std::move(clip));
                anyAdded = true;
            }
        }

        if (!anyAdded) {
            Logger::Error("AddAnimationFromFile failed: no channels matched skeleton node names.");
        }

        return anyAdded;
    }

} // namespace Engine
