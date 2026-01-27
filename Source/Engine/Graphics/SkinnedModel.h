#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

#include <DirectXMath.h>

#include "Engine/Graphics/Mesh.h"
#include "Engine/Graphics/Material.h"
#include "Engine/Graphics/ShaderConstants.h"

namespace Engine {

    struct SkinnedVertex final {
        DirectX::XMFLOAT3 m_position{};
        DirectX::XMFLOAT3 m_normal{};
        DirectX::XMFLOAT4 m_color{ 1,1,1,1 };
        DirectX::XMFLOAT2 m_uv{ 0,0 };

        std::uint32_t m_boneIndices[4]{ 0,0,0,0 };
        float m_boneWeights[4]{ 0,0,0,0 };
    };

    /**
     * @brief 拡張スキニング頂点（PBR対応：Tangent/Bitangent含む）
     */
    struct ExtendedSkinnedVertex final {
        DirectX::XMFLOAT3 m_position{};
        DirectX::XMFLOAT3 m_normal{};
        DirectX::XMFLOAT3 m_tangent{};
        DirectX::XMFLOAT3 m_bitangent{};
        DirectX::XMFLOAT2 m_uv{ 0,0 };

        std::uint32_t m_boneIndices[4]{ 0,0,0,0 };
        float m_boneWeights[4]{ 0,0,0,0 };
    };

    /**
     * @brief ボーン変換データ（位置、回転、スケール）
     */
    struct BoneTransform final {
        DirectX::XMFLOAT3 m_position{ 0,0,0 };
        DirectX::XMFLOAT4 m_rotation{ 0,0,0,1 }; // quaternion
        DirectX::XMFLOAT3 m_scale{ 1,1,1 };

        /// @brief 行列に変換
        DirectX::XMMATRIX ToMatrix() const;
    };

    struct SkinnedSubset final {
        std::uint32_t m_startIndex = 0;
        std::uint32_t m_indexCount = 0;
        std::shared_ptr<Material> m_material;
    };

    struct SkinnedNode final {
        std::string m_name{};
        int m_parent = -1;
        std::vector<int> m_children;
        DirectX::XMFLOAT4X4 m_defaultLocal{};
    };

    struct SkinnedChannel final {
        std::vector<DirectX::XMFLOAT3> m_positions;
        std::vector<DirectX::XMFLOAT4> m_rotations; // quaternion(x,y,z,w)
        std::vector<DirectX::XMFLOAT3> m_scales;

        /// @brief チャンネル内の最大キーフレーム数を取得
        int GetMaxKeyCount() const {
            int maxCount = static_cast<int>(m_positions.size());
            if (static_cast<int>(m_rotations.size()) > maxCount) maxCount = static_cast<int>(m_rotations.size());
            if (static_cast<int>(m_scales.size()) > maxCount) maxCount = static_cast<int>(m_scales.size());
            return maxCount;
        }
    };

    struct SkinnedClip final {
        // key: nodeIndex
        std::unordered_map<int, SkinnedChannel> m_channels;
        float m_duration = 0.0f;       ///< クリップの長さ（秒）
        float m_ticksPerSecond = 30.0f; ///< 1秒あたりのティック数

        /// @brief クリップ内の最大キーフレーム数を取得
        int GetMaxKeyCount() const {
            int maxCount = 0;
            for (const auto& pair : m_channels) {
                int count = pair.second.GetMaxKeyCount();
                if (count > maxCount) maxCount = count;
            }
            return maxCount;
        }
    };

    struct SkinnedBone final {
        std::string m_name{};
        int m_nodeIndex = -1;
        DirectX::XMFLOAT4X4 m_offset{};
    };

    class SkinnedModel final {
    public:
        SkinnedModel() = default;
        ~SkinnedModel() = default;

        SkinnedModel(const SkinnedModel&) = delete;
        SkinnedModel& operator=(const SkinnedModel&) = delete;

        bool Initialize(
            ID3D11Device* device,
            const std::vector<SkinnedVertex>& vertices,
            const std::vector<std::uint32_t>& indices,
            std::vector<SkinnedSubset>&& subsets,
            std::vector<SkinnedNode>&& nodes,
            std::vector<SkinnedBone>&& bones,
            std::unordered_map<std::string, SkinnedClip>&& clips,
            int rootNodeIndex
        );

        bool IsValid() const;

        Mesh& GetMesh();
        const Mesh& GetMesh() const;

        const std::vector<SkinnedSubset>& GetSubsets() const;
        const std::vector<SkinnedBone>& GetBones() const;
        const std::vector<SkinnedNode>& GetNodes() const;

        std::vector<std::string> GetClipNames() const;

        /// @brief クリップのフレーム数を取得
        int GetClipFrameCount(const std::string& clipName) const;

        /// @brief クリップを取得（存在しない場合はnullptr）
        const SkinnedClip* GetClip(const std::string& clipName) const;

        // 別アニメファイル対応：外からクリップ追加できるようにする
        bool AddClip(const std::string& clipName, SkinnedClip&& clip);

        // 別アニメファイルからnodeをマッピングするために使う
        int FindNodeIndexByName(const std::string& nodeName) const;

        /// @brief 時間ベースのアニメーション評価（補間あり）
        bool EvaluateTime(
            const std::string& clipName, float timeInSeconds,
            std::vector<DirectX::XMFLOAT4X4>& outBoneMatrices
        ) const;

        /// @brief 2つのクリップ間の時間ベースブレンド評価
        bool EvaluateBlendTime(
            const std::string& clipA, float timeA,
            const std::string& clipB, float timeB,
            float blendRate,
            std::vector<DirectX::XMFLOAT4X4>& outBoneMatrices
        ) const;

        bool EvaluateBlendFrames(
            const std::string& clipA, int frameA,
            const std::string& clipB, int frameB,
            float blendRate,
            std::vector<DirectX::XMFLOAT4X4>& outBoneMatrices
        ) const;

        bool EvaluateFrames(
            const std::string& clipName, int frame,
            std::vector<DirectX::XMFLOAT4X4>& outBoneMatrices
        ) const;

    private:
        DirectX::XMMATRIX BuildLocalMatrix(
            const DirectX::XMFLOAT3& scale,
            const DirectX::XMFLOAT4& rotation,
            const DirectX::XMFLOAT3& position
        ) const;

    private:
        Mesh m_mesh;
        std::vector<SkinnedSubset> m_subsets;

        std::vector<SkinnedNode> m_nodes;
        std::vector<SkinnedBone> m_bones;

        std::unordered_map<std::string, SkinnedClip> m_clips;

        // ��node����index�i�ǉ��A�j����荞�݂ŕK�v�j
        std::unordered_map<std::string, int> m_nodeNameToIndex;

        int m_rootNodeIndex = 0;
    };

} // namespace Engine
