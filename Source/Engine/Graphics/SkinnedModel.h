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

    /// @brief スキニング頂点（位置、法線、色、UV、ボーンインデックス/ウェイト）
    struct SkinnedVertex final {
        DirectX::XMFLOAT3 m_position{};
        DirectX::XMFLOAT3 m_normal{};
        DirectX::XMFLOAT4 m_color{ 1,1,1,1 };
        DirectX::XMFLOAT2 m_uv{ 0,0 };

        std::uint32_t m_boneIndices[4]{ 0,0,0,0 };
        float m_boneWeights[4]{ 0,0,0,0 };
    };

	/// @brief スキニング頂点（位置、法線、タンジェント、ビタジェント、UV、ボーンインデックス/ウェイト）
    struct ExtendedSkinnedVertex final {
        DirectX::XMFLOAT3 m_position{};
        DirectX::XMFLOAT3 m_normal{};
        DirectX::XMFLOAT3 m_tangent{};
        DirectX::XMFLOAT3 m_bitangent{};
        DirectX::XMFLOAT2 m_uv{ 0,0 };

        std::uint32_t m_boneIndices[4]{ 0,0,0,0 };
        float m_boneWeights[4]{ 0,0,0,0 };
    };

	/// @brief ボーン変換（位置、回転、スケール）
    struct BoneTransform final {
        DirectX::XMFLOAT3 m_position{ 0,0,0 };
        DirectX::XMFLOAT4 m_rotation{ 0,0,0,1 }; // quaternion
        DirectX::XMFLOAT3 m_scale{ 1,1,1 };

        /// @brief 行列に変換
        DirectX::XMMATRIX ToMatrix() const;
    };

	/// @brief サブセット（インデックスの範囲とマテリアル）
    struct SkinnedSubset final {
        std::uint32_t m_startIndex = 0;
        std::uint32_t m_indexCount = 0;
        std::shared_ptr<Material> m_material;
    };

	/// @brief ノード（階層構造とデフォルトローカル行列）
    struct SkinnedNode final {
        std::string m_name{};
        int m_parent = -1;
        std::vector<int> m_children;
        DirectX::XMFLOAT4X4 m_defaultLocal{};
    };

	/// @brief チャンネル（位置/回転/スケールのキーフレーム）
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

	/// @brief クリップ（複数のチャンネルと再生情報）
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

	/// @brief ボーン（ノードへの参照とオフセット行列）
    struct SkinnedBone final {
        std::string m_name{};
        int m_nodeIndex = -1;
        DirectX::XMFLOAT4X4 m_offset{};
    };

	/// @brief スキニングモデル（メッシュ、サブセット、ノード、ボーン、クリップを管理）
	/// @note クリップはノードインデックスをキーとするチャンネルの集合で、各チャンネルは位置/回転/スケールのキーフレームを持つ
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

		/// @brief サブセットを取得
		/// @return サブセットのベクターへのconst参照
        const std::vector<SkinnedSubset>&   GetSubsets() const;
		/// @brief ボーンを取得
		/// @return ボーンのベクターへのconst参照
        const std::vector<SkinnedBone>&     GetBones() const;
		/// @brief ノードを取得
		/// @return ノードのベクターへのconst参照
        const std::vector<SkinnedNode>&     GetNodes() const;

		/// @brief クリップ名のリストを取得
		/// @return クリップ名のベクター
        std::vector<std::string>            GetClipNames() const;

        /// @brief クリップのフレーム数を取得
		/// @param clipName クリップ名
		/// @return フレーム数（クリップが存在しない場合は0）
        int GetClipFrameCount(const std::string& clipName) const;

        /// @brief クリップを取得（存在しない場合はnullptr）
		/// @param clipName クリップ名
        /// @return クリップへのconstポインタ（存在しない場合はnullptr）
        const SkinnedClip* GetClip(const std::string& clipName) const;

        /// @brief 別アニメファイル対応：外からクリップ追加できるようにする
		/// @param clipName クリップ名
        /// @param clip 追加するクリップ
        /// @return 成功した場合はtrue、失敗した場合はfalse
        bool AddClip(const std::string& clipName, SkinnedClip&& clip);

        /// @brief 別アニメファイルからnodeをマッピングするために使う
		/// @param nodeName ノード名
        /// @return ノードのインデックス（存在しない場合は-1）
        int FindNodeIndexByName(const std::string& nodeName) const;

        /// @brief 時間ベースのアニメーション評価（補間あり）
		/// @param clipName クリップ名
        /// @param timeInSeconds 評価する時間（秒）
        /// @param outBoneMatrices 評価結果のボーン行列
        /// @return 成功した場合はtrue、失敗した場合はfalse
        bool EvaluateTime(
            const std::string& clipName, float timeInSeconds,
            std::vector<DirectX::XMFLOAT4X4>& outBoneMatrices
        ) const;

        /// @brief 2つのクリップ間の時間ベースブレンド評価
		/// @param clipA クリップAの名前
		/// @param timeA クリップAの評価時間（秒）
		/// @param clipB クリップBの名前
		/// @param timeB クリップBの評価時間（秒）
        /// @param blendRate ブレンド率
        /// @param outBoneMatrices 評価結果のボーン行列
        /// @return 成功した場合はtrue、失敗した場合はfalse
        bool EvaluateBlendTime(
            const std::string& clipA, float timeA,
            const std::string& clipB, float timeB,
            float blendRate,
            std::vector<DirectX::XMFLOAT4X4>& outBoneMatrices
        ) const;

		/// @brief フレームベースのアニメーション評価（補間なし）
		/// @param clipA クリップAの名前
        /// @param frameA フレーム番号
		/// @param clipB クリップBの名前
        /// @param frameB フレーム番号
        /// @param blendRate ブレンド率
        /// @param outBoneMatrices 評価結果のボーン行列
        /// @return 成功した場合はtrue、失敗した場合はfalse
        bool EvaluateBlendFrames(
            const std::string& clipA, int frameA,
            const std::string& clipB, int frameB,
            float blendRate,
            std::vector<DirectX::XMFLOAT4X4>& outBoneMatrices
        ) const;

		/// @brief フレームベースのアニメーション評価（補間なし）
		/// @param clipName クリップ名
        /// @param frame フレーム番号
        /// @param outBoneMatrices 評価結果のボーン行列
        /// @return 成功した場合はtrue、失敗した場合はfalse
        bool EvaluateFrames(
            const std::string& clipName, int frame,
            std::vector<DirectX::XMFLOAT4X4>& outBoneMatrices
        ) const;

    private:
		/// @brief ローカル変換（スケール、回転、位置）からローカル行列を構築
		/// @param scale スケール
        /// @param rotation 回転
        /// @param position 位置
        /// @return ローカル行列
        DirectX::XMMATRIX BuildLocalMatrix(
            const DirectX::XMFLOAT3& scale,
            const DirectX::XMFLOAT4& rotation,
            const DirectX::XMFLOAT3& position
        ) const;

    private:
		Mesh m_mesh;                            /// メッシュ（頂点バッファ、インデックスバッファを管理）
		std::vector<SkinnedSubset>  m_subsets;  /// サブセット（インデックスの範囲とマテリアルを管理）

		std::vector<SkinnedNode>    m_nodes;    /// ノード（階層構造とデフォルトローカル行列を管理）
		std::vector<SkinnedBone>    m_bones;    /// ボーン（ノードへの参照とオフセット行列を管理）

		std::unordered_map<std::string, SkinnedClip> m_clips;   /// クリップ（複数のチャンネルと再生情報を管理）

		// ノード名からインデックスへのマッピング（外部からのアクセス用）
		std::unordered_map<std::string, int> m_nodeNameToIndex; /// ノード名からインデックスへのマッピング

		int m_rootNodeIndex = 0;                /// ルートノードのインデックス
    };

} // namespace Engine
