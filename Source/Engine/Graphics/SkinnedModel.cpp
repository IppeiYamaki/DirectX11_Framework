#include "SkinnedModel.h"

#include "Engine/Core/Logger.h"

namespace Engine {

    bool SkinnedModel::Initialize(
        ID3D11Device* device,
        const std::vector<SkinnedVertex>& vertices,
        const std::vector<std::uint32_t>& indices,
        std::vector<SkinnedSubset>&& subsets,
        std::vector<SkinnedNode>&& nodes,
        std::vector<SkinnedBone>&& bones,
        std::unordered_map<std::string, SkinnedClip>&& clips,
        int rootNodeIndex
    ) {
        if (!device) {
            Logger::Error("SkinnedModel::Initialize failed: device is null.");
            return false;
        }
        if (vertices.empty() || indices.empty()) {
            Logger::Error("SkinnedModel::Initialize failed: mesh data empty.");
            return false;
        }

        m_mesh.Destroy();
        if (!m_mesh.Create(
            device,
            vertices.data(),
            static_cast<std::uint32_t>(sizeof(SkinnedVertex)),
            static_cast<std::uint32_t>(vertices.size()),
            indices.data(),
            static_cast<std::uint32_t>(indices.size())
        )) {
            Logger::Error("SkinnedModel::Initialize failed: Mesh::Create failed.");
            return false;
        }

        m_subsets = std::move(subsets);
        m_nodes = std::move(nodes);
        m_bones = std::move(bones);
        m_clips = std::move(clips);
        m_rootNodeIndex = rootNodeIndex;

        // node����index
        m_nodeNameToIndex.clear();
        m_nodeNameToIndex.reserve(m_nodes.size());
        for (int i = 0; i < static_cast<int>(m_nodes.size()); ++i) {
            m_nodeNameToIndex[m_nodes[i].m_name] = i;
        }

        // subsets����Ȃ�S��
        if (m_subsets.empty()) {
            SkinnedSubset s{};
            s.m_startIndex = 0;
            s.m_indexCount = m_mesh.GetIndexCount();
            s.m_material = nullptr;
            m_subsets.push_back(std::move(s));
        }

        return true;
    }

    bool SkinnedModel::IsValid() const {
        return m_mesh.IsValid() && !m_bones.empty();
    }

    Mesh& SkinnedModel::GetMesh() { return m_mesh; }
    const Mesh& SkinnedModel::GetMesh() const { return m_mesh; }

    const std::vector<SkinnedSubset>& SkinnedModel::GetSubsets() const { return m_subsets; }
    const std::vector<SkinnedBone>& SkinnedModel::GetBones() const { return m_bones; }
    const std::vector<SkinnedNode>& SkinnedModel::GetNodes() const { return m_nodes; }

    std::vector<std::string> SkinnedModel::GetClipNames() const {
        std::vector<std::string> names;
        names.reserve(m_clips.size());
        for (const auto& kv : m_clips) names.push_back(kv.first);
        return names;
    }

    int SkinnedModel::GetClipFrameCount(const std::string& clipName) const {
        auto it = m_clips.find(clipName);
        if (it == m_clips.end()) return 0;
        return it->second.GetMaxKeyCount();
    }

    const SkinnedClip* SkinnedModel::GetClip(const std::string& clipName) const {
        auto it = m_clips.find(clipName);
        if (it == m_clips.end()) return nullptr;
        return &it->second;
    }

    bool SkinnedModel::AddClip(const std::string& clipName, SkinnedClip&& clip) {
        if (clipName.empty()) return false;
        m_clips[clipName] = std::move(clip);
        return true;
    }

    int SkinnedModel::FindNodeIndexByName(const std::string& nodeName) const {
        auto it = m_nodeNameToIndex.find(nodeName);
        if (it == m_nodeNameToIndex.end()) return -1;
        return it->second;
    }

    DirectX::XMMATRIX SkinnedModel::BuildLocalMatrix(
        const DirectX::XMFLOAT3& scale,
        const DirectX::XMFLOAT4& rotation,
        const DirectX::XMFLOAT3& position
    ) const {
        using namespace DirectX;

        const XMVECTOR s = XMVectorSet(scale.x, scale.y, scale.z, 0.0f);
        const XMVECTOR q = XMQuaternionNormalize(XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w));
        const XMVECTOR t = XMVectorSet(position.x, position.y, position.z, 0.0f);

        const XMMATRIX ms = XMMatrixScalingFromVector(s);
        const XMMATRIX mr = XMMatrixRotationQuaternion(q);
        const XMMATRIX mt = XMMatrixTranslationFromVector(t);

        return XMMatrixMultiply(XMMatrixMultiply(ms, mr), mt);
    }

    // ������frame�ł����S�� index �����
    static int WrapFrameIndex(int frame, int keyCount) {
        if (keyCount <= 0) return 0;
        int f = frame % keyCount;
        if (f < 0) f += keyCount;
        return f;
    }

    static DirectX::XMFLOAT3 SampleVec3(
        const std::vector<DirectX::XMFLOAT3>& keys,
        int frame,
        const DirectX::XMFLOAT3& fallback
    ) {
        if (keys.empty()) return fallback;

        const int n = static_cast<int>(keys.size());
        const int f = WrapFrameIndex(frame, n);
        return keys[static_cast<size_t>(f)];
    }

    static DirectX::XMFLOAT4 SampleQuat(
        const std::vector<DirectX::XMFLOAT4>& keys,
        int frame,
        const DirectX::XMFLOAT4& fallback
    ) {
        if (keys.empty()) return fallback;

        const int n = static_cast<int>(keys.size());
        const int f = WrapFrameIndex(frame, n);
        return keys[static_cast<size_t>(f)];
    }

    bool SkinnedModel::EvaluateFrames(
        const std::string& clipName, int frame,
        std::vector<DirectX::XMFLOAT4X4>& outBoneMatrices
    ) const {
        return EvaluateBlendFrames(clipName, frame, clipName, frame, 0.0f, outBoneMatrices);
    }

    bool SkinnedModel::EvaluateBlendFrames(
        const std::string& clipA, int frameA,
        const std::string& clipB, int frameB,
        float blendRate,
        std::vector<DirectX::XMFLOAT4X4>& outBoneMatrices
    ) const {
        if (m_nodes.empty() || m_bones.empty()) return false;

        auto itA = m_clips.find(clipA);
        auto itB = m_clips.find(clipB);
        if (itA == m_clips.end() || itB == m_clips.end()) return false;

        const SkinnedClip& a = itA->second;
        const SkinnedClip& b = itB->second;

        const float t = (blendRate < 0.0f) ? 0.0f : (blendRate > 1.0f ? 1.0f : blendRate);

        std::vector<DirectX::XMMATRIX> local(m_nodes.size());
        std::vector<DirectX::XMMATRIX> global(m_nodes.size());

        for (size_t i = 0; i < m_nodes.size(); ++i) {
            local[i] = DirectX::XMLoadFloat4x4(&m_nodes[i].m_defaultLocal);
        }

        const DirectX::XMFLOAT4 identityQ{ 0,0,0,1 };
        const DirectX::XMFLOAT3 oneS{ 1,1,1 };
        const DirectX::XMFLOAT3 zeroT{ 0,0,0 };

        for (size_t nodeIndex = 0; nodeIndex < m_nodes.size(); ++nodeIndex) {
            auto itChA = a.m_channels.find(static_cast<int>(nodeIndex));
            auto itChB = b.m_channels.find(static_cast<int>(nodeIndex));

            const SkinnedChannel* chA = (itChA != a.m_channels.end()) ? &itChA->second : nullptr;
            const SkinnedChannel* chB = (itChB != b.m_channels.end()) ? &itChB->second : nullptr;

            if (!chA && !chB) continue;

            DirectX::XMFLOAT3 posA = chA ? SampleVec3(chA->m_positions, frameA, zeroT) : zeroT;
            DirectX::XMFLOAT3 sclA = chA ? SampleVec3(chA->m_scales, frameA, oneS) : oneS;
            DirectX::XMFLOAT4 rotA = chA ? SampleQuat(chA->m_rotations, frameA, identityQ) : identityQ;

            DirectX::XMFLOAT3 posB = chB ? SampleVec3(chB->m_positions, frameB, zeroT) : posA;
            DirectX::XMFLOAT3 sclB = chB ? SampleVec3(chB->m_scales, frameB, oneS) : sclA;
            DirectX::XMFLOAT4 rotB = chB ? SampleQuat(chB->m_rotations, frameB, identityQ) : rotA;

            using namespace DirectX;
            const XMVECTOR vPos = XMVectorLerp(XMLoadFloat3(&posA), XMLoadFloat3(&posB), t);
            const XMVECTOR vScl = XMVectorLerp(XMLoadFloat3(&sclA), XMLoadFloat3(&sclB), t);
            const XMVECTOR vRot = XMQuaternionSlerp(XMLoadFloat4(&rotA), XMLoadFloat4(&rotB), t);

            XMFLOAT3 pos{};
            XMFLOAT3 scl{};
            XMFLOAT4 rot{};
            XMStoreFloat3(&pos, vPos);
            XMStoreFloat3(&scl, vScl);
            XMStoreFloat4(&rot, vRot);

            local[nodeIndex] = BuildLocalMatrix(scl, rot, pos);
        }

        std::function<void(int, const DirectX::XMMATRIX&)> build =
            [&](int idx, const DirectX::XMMATRIX& parent) {
            global[idx] = DirectX::XMMatrixMultiply(local[idx], parent);
            for (int c : m_nodes[idx].m_children) {
                build(c, global[idx]);
            }
            };

        build(m_rootNodeIndex, DirectX::XMMatrixIdentity());

        outBoneMatrices.resize(m_bones.size());
        for (size_t i = 0; i < m_bones.size(); ++i) {
            const SkinnedBone& bone = m_bones[i];
            if (bone.m_nodeIndex < 0 || bone.m_nodeIndex >= static_cast<int>(global.size())) {
                DirectX::XMStoreFloat4x4(&outBoneMatrices[i], DirectX::XMMatrixIdentity());
                continue;
            }

            const DirectX::XMMATRIX g = global[bone.m_nodeIndex];
            const DirectX::XMMATRIX off = DirectX::XMLoadFloat4x4(&bone.m_offset);

            const DirectX::XMMATRIX finalM = DirectX::XMMatrixMultiply(g, off);
            DirectX::XMStoreFloat4x4(&outBoneMatrices[i], finalM);
        }

        return true;
    }

    bool SkinnedModel::EvaluateTime(
        const std::string& clipName, float timeInSeconds,
        std::vector<DirectX::XMFLOAT4X4>& outBoneMatrices
    ) const {
        auto it = m_clips.find(clipName);
        if (it == m_clips.end()) return false;

        const SkinnedClip& clip = it->second;
        const int maxKeys = clip.GetMaxKeyCount();
        if (maxKeys <= 0) return false;

        // 時間からフレームを計算（補間付き）
        float ticksPerSecond = clip.m_ticksPerSecond > 0.0f ? clip.m_ticksPerSecond : 30.0f;
        float frameFloat = timeInSeconds * ticksPerSecond;

        int frame = static_cast<int>(frameFloat);
        if (frame < 0) frame = 0;
        if (frame >= maxKeys) frame = maxKeys - 1;

        return EvaluateFrames(clipName, frame, outBoneMatrices);
    }

    bool SkinnedModel::EvaluateBlendTime(
        const std::string& clipA, float timeA,
        const std::string& clipB, float timeB,
        float blendRate,
        std::vector<DirectX::XMFLOAT4X4>& outBoneMatrices
    ) const {
        auto itA = m_clips.find(clipA);
        auto itB = m_clips.find(clipB);
        if (itA == m_clips.end() || itB == m_clips.end()) return false;

        const SkinnedClip& a = itA->second;
        const SkinnedClip& b = itB->second;

        // 時間からフレームを計算
        float ticksA = a.m_ticksPerSecond > 0.0f ? a.m_ticksPerSecond : 30.0f;
        float ticksB = b.m_ticksPerSecond > 0.0f ? b.m_ticksPerSecond : 30.0f;

        int frameA = static_cast<int>(timeA * ticksA);
        int frameB = static_cast<int>(timeB * ticksB);

        const int maxA = a.GetMaxKeyCount();
        const int maxB = b.GetMaxKeyCount();

        if (frameA < 0) frameA = 0;
        if (maxA > 0 && frameA >= maxA) frameA = maxA - 1;
        if (frameB < 0) frameB = 0;
        if (maxB > 0 && frameB >= maxB) frameB = maxB - 1;

        return EvaluateBlendFrames(clipA, frameA, clipB, frameB, blendRate, outBoneMatrices);
    }

    // BoneTransform implementation
    DirectX::XMMATRIX BoneTransform::ToMatrix() const {
        using namespace DirectX;

        const XMVECTOR s = XMVectorSet(m_scale.x, m_scale.y, m_scale.z, 0.0f);
        const XMVECTOR q = XMQuaternionNormalize(XMVectorSet(m_rotation.x, m_rotation.y, m_rotation.z, m_rotation.w));
        const XMVECTOR t = XMVectorSet(m_position.x, m_position.y, m_position.z, 0.0f);

        const XMMATRIX ms = XMMatrixScalingFromVector(s);
        const XMMATRIX mr = XMMatrixRotationQuaternion(q);
        const XMMATRIX mt = XMMatrixTranslationFromVector(t);

        return XMMatrixMultiply(XMMatrixMultiply(ms, mr), mt);
    }

} // namespace Engine
