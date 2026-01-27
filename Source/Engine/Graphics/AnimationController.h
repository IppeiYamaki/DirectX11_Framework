#pragma once

#include <string>
#include <vector>

#include <DirectXMath.h>

namespace Engine {

    class SkinnedModel;

    /**
     * @brief アニメーションコントローラ
     * 
     * SkinnedModelのアニメーションクリップを管理し、
     * スムーズなアニメーションブレンドと遷移を可能にする。
     */
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
        float GetCurrentTime() const { return m_currentTime; }

        /// @brief 現在の再生速度を取得
        float GetPlaybackSpeed() const { return m_playbackSpeed; }

        /// @brief 再生速度を設定
        void SetPlaybackSpeed(float speed) { m_playbackSpeed = speed; }

        /// @brief 現在再生中のクリップ名を取得
        const std::string& GetCurrentClipName() const { return m_currentClipName; }

        /// @brief ブレンド中かどうか
        bool IsBlending() const { return m_isBlending; }

        /// @brief 再生中かどうか
        bool IsPlaying() const { return m_isPlaying; }

        /// @brief 現在のブレンド係数を取得
        float GetBlendFactor() const { return m_blendFactor; }

        /// @brief 計算済みのボーン行列を取得
        const std::vector<DirectX::XMFLOAT4X4>& GetBoneMatrices() const { return m_finalBoneMatrices; }

        /// @brief 指定インデックスのボーン行列を取得
        DirectX::XMFLOAT4X4 GetBoneMatrix(int boneIndex) const;

    private:
        /// @brief ボーン行列を計算
        void CalculateBoneMatrices();

    private:
        const SkinnedModel* m_model = nullptr;

        std::string m_currentClipName{};
        std::string m_nextClipName{};

        float m_currentTime = 0.0f;
        float m_nextTime = 0.0f;
        float m_playbackSpeed = 1.0f;

        bool m_isPlaying = false;
        bool m_isLooping = true;
        bool m_isBlending = false;

        float m_blendTime = 0.0f;
        float m_blendElapsed = 0.0f;
        float m_blendFactor = 0.0f;

        bool m_nextLooping = true;

        std::vector<DirectX::XMFLOAT4X4> m_finalBoneMatrices;
    };

} // namespace Engine
