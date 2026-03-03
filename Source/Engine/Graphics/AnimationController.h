#pragma once

#include <string>
#include <vector>

#include <DirectXMath.h>

namespace Engine {

    class SkinnedModel;

    /// @brief アニメーションコントローラ
	/// @brief SkinnedModelのアニメーションクリップを管理し、スムーズなアニメーションブレンドと遷移を可能にする
    class AnimationController final {
    public:
        AnimationController() = default;
        ~AnimationController() = default;

        AnimationController(const AnimationController&) = delete;
        AnimationController& operator=(const AnimationController&) = delete;

        /// @brief コントローラを初期化
        /// @param model 対象のSkinnedModel
        void Initialize(const SkinnedModel* model);

        /// @brief 新しいアニメーションを再生
        /// @param clipName クリップ名
        /// @param blendTime ブレンド時間（秒）。0の場合は即座に切り替え
        /// @param looping ループ再生するか
        void PlayAnimation(const std::string& clipName, float blendTime = 0.5f, bool looping = true);

        /// @brief 現在のアニメーションを停止
        void Stop();

        /// @brief アニメーションを更新
        /// @param deltaTime 経過時間（秒）
        void Update(float deltaTime);

        /// @brief 現在の再生時間を取得
		/// @return 再生時間（秒）
        float GetCurrentTime() const { return m_currentTime; }

        /// @brief 現在の再生速度を取得
		/// @return 再生速度（1.0が通常速度）
        float GetPlaybackSpeed() const { return m_playbackSpeed; }

        /// @brief 再生速度を設定
        /// @param speed 再生速度（1.0が通常速度）
        void SetPlaybackSpeed(float speed) { m_playbackSpeed = speed; }

        /// @brief 現在再生中のクリップ名を取得
		/// @return クリップ名（再生していない場合は空文字列）
        const std::string& GetCurrentClipName() const { return m_currentClipName; }

        /// @brief ブレンド中かどうか
		/// @return ブレンド中ならtrue
        bool IsBlending() const { return m_isBlending; }

        /// @brief 再生中かどうか
		/// @return 再生中ならtrue
        bool IsPlaying() const { return m_isPlaying; }

        /// @brief 現在のブレンド係数を取得
		/// @return ブレンド係数（0.0が現在のアニメーション、1.0が次のアニメーション）
        float GetBlendFactor() const { return m_blendFactor; }

        /// @brief 計算済みのボーン行列を取得
		/// @return ボーン行列のベクター（サイズはモデルのボーン数と同じ）
        const std::vector<DirectX::XMFLOAT4X4>& GetBoneMatrices() const { return m_finalBoneMatrices; }

        /// @brief 指定インデックスのボーン行列を取得
		/// @param boneIndex ボーンインデックス
        DirectX::XMFLOAT4X4 GetBoneMatrix(int boneIndex) const;

    private:
        /// @brief ボーン行列を計算
        void CalculateBoneMatrices();

    private:
		const SkinnedModel* m_model = nullptr;  // 対象のSkinnedModel

		std::string m_currentClipName{};        // 現在のクリップ名（再生していない場合は空文字列）
		std::string m_nextClipName{};           // 次のクリップ名（ブレンド中でない場合は空文字列）

        float m_currentTime     = 0.0f;         // 現在の再生時間（秒）
        float m_nextTime        = 0.0f;         // 次のクリップの再生時間（秒）
        float m_playbackSpeed   = 1.0f;         // 再生速度（1.0が通常速度）

        bool m_isPlaying        = false;        // 再生中フラグ
        bool m_isLooping        = true;         // 次のクリップのループ設定
        bool m_isBlending       = false;        // ブレンド中フラグ

        float m_blendTime       = 0.0f;         // ブレンド時間（秒）
        float m_blendElapsed    = 0.0f;         // 経過時間（秒）
        float m_blendFactor     = 0.0f;         // ブレンド係数（0.0が現在のアニメーション、1.0が次のアニメーション）

        bool m_nextLooping      = true;         // 次のクリップのループ設定

		std::vector<DirectX::XMFLOAT4X4> m_finalBoneMatrices;   // 計算済みのボーン行列（サイズはモデルのボーン数と同じ）
    };

} // namespace Engine
