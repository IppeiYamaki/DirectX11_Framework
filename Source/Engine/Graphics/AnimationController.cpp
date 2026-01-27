#include "AnimationController.h"

#include "Engine/Graphics/SkinnedModel.h"
#include "Engine/Core/Logger.h"

namespace Engine {

    void AnimationController::Initialize(const SkinnedModel* model) {
        m_model = model;
        m_currentClipName.clear();
        m_nextClipName.clear();
        m_currentTime = 0.0f;
        m_nextTime = 0.0f;
        m_isPlaying = false;
        m_isBlending = false;
        m_blendFactor = 0.0f;
        m_finalBoneMatrices.clear();

        if (m_model) {
            m_finalBoneMatrices.resize(m_model->GetBones().size());
            for (auto& mat : m_finalBoneMatrices) {
                DirectX::XMStoreFloat4x4(&mat, DirectX::XMMatrixIdentity());
            }
        }
    }

    void AnimationController::PlayAnimation(const std::string& clipName, float blendTime, bool looping) {
        if (!m_model) {
            Logger::Warn("AnimationController::PlayAnimation: model is null.");
            return;
        }

        // 同じアニメーションを再生中なら何もしない
        if (m_currentClipName == clipName && m_isPlaying && !m_isBlending) {
            return;
        }

        // クリップの存在確認
        if (!m_model->GetClip(clipName)) {
            Logger::Warn("AnimationController::PlayAnimation: clip not found.");
            return;
        }

        if (blendTime > 0.0f && m_isPlaying && !m_currentClipName.empty()) {
            // ブレンド開始
            m_nextClipName = clipName;
            m_nextTime = 0.0f;
            m_nextLooping = looping;
            m_blendTime = blendTime;
            m_blendElapsed = 0.0f;
            m_blendFactor = 0.0f;
            m_isBlending = true;
        }
        else {
            // 即座に切り替え
            m_currentClipName = clipName;
            m_currentTime = 0.0f;
            m_isLooping = looping;
            m_isPlaying = true;
            m_isBlending = false;
            m_blendFactor = 0.0f;
        }
    }

    void AnimationController::Stop() {
        m_isPlaying = false;
        m_isBlending = false;
        m_currentTime = 0.0f;
        m_nextTime = 0.0f;
    }

    void AnimationController::Update(float deltaTime) {
        if (!m_model || !m_isPlaying) {
            return;
        }

        const float adjustedDelta = deltaTime * m_playbackSpeed;

        if (m_isBlending) {
            // ブレンド処理
            m_blendElapsed += deltaTime;
            m_blendFactor = m_blendElapsed / m_blendTime;

            if (m_blendFactor >= 1.0f) {
                // ブレンド完了
                m_blendFactor = 1.0f;
                m_currentClipName = m_nextClipName;
                m_currentTime = m_nextTime;
                m_isLooping = m_nextLooping;
                m_nextClipName.clear();
                m_isBlending = false;
            }

            // 両方のアニメーション時間を進める
            m_currentTime += adjustedDelta;
            m_nextTime += adjustedDelta;

            // クリップの長さでラップ
            const SkinnedClip* currentClip = m_model->GetClip(m_currentClipName);
            const SkinnedClip* nextClip = m_model->GetClip(m_nextClipName);

            if (currentClip) {
                float clipDuration = currentClip->m_duration;
                if (clipDuration <= 0.0f) {
                    float ticksPerSecond = currentClip->m_ticksPerSecond > 0.0f ? currentClip->m_ticksPerSecond : 30.0f;
                    clipDuration = static_cast<float>(currentClip->GetMaxKeyCount()) / ticksPerSecond;
                }
                if (clipDuration > 0.0f && m_isLooping) {
                    while (m_currentTime >= clipDuration) {
                        m_currentTime -= clipDuration;
                    }
                }
            }

            if (nextClip) {
                float clipDuration = nextClip->m_duration;
                if (clipDuration <= 0.0f) {
                    float ticksPerSecond = nextClip->m_ticksPerSecond > 0.0f ? nextClip->m_ticksPerSecond : 30.0f;
                    clipDuration = static_cast<float>(nextClip->GetMaxKeyCount()) / ticksPerSecond;
                }
                if (clipDuration > 0.0f && m_nextLooping) {
                    while (m_nextTime >= clipDuration) {
                        m_nextTime -= clipDuration;
                    }
                }
            }
        }
        else {
            // 通常再生
            m_currentTime += adjustedDelta;

            const SkinnedClip* currentClip = m_model->GetClip(m_currentClipName);
            if (currentClip) {
                float clipDuration = currentClip->m_duration;
                if (clipDuration <= 0.0f) {
                    float ticksPerSecond = currentClip->m_ticksPerSecond > 0.0f ? currentClip->m_ticksPerSecond : 30.0f;
                    clipDuration = static_cast<float>(currentClip->GetMaxKeyCount()) / ticksPerSecond;
                }

                if (clipDuration > 0.0f) {
                    if (m_isLooping) {
                        while (m_currentTime >= clipDuration) {
                            m_currentTime -= clipDuration;
                        }
                    }
                    else if (m_currentTime >= clipDuration) {
                        m_currentTime = clipDuration;
                        m_isPlaying = false;
                    }
                }
            }
        }

        // ボーン行列を計算
        CalculateBoneMatrices();
    }

    void AnimationController::CalculateBoneMatrices() {
        if (!m_model) return;

        if (m_isBlending && !m_nextClipName.empty()) {
            // ブレンド評価
            m_model->EvaluateBlendTime(
                m_currentClipName, m_currentTime,
                m_nextClipName, m_nextTime,
                m_blendFactor,
                m_finalBoneMatrices
            );
        }
        else {
            // 単一クリップ評価
            m_model->EvaluateTime(
                m_currentClipName, m_currentTime,
                m_finalBoneMatrices
            );
        }
    }

    DirectX::XMFLOAT4X4 AnimationController::GetBoneMatrix(int boneIndex) const {
        if (boneIndex < 0 || boneIndex >= static_cast<int>(m_finalBoneMatrices.size())) {
            DirectX::XMFLOAT4X4 identity{};
            DirectX::XMStoreFloat4x4(&identity, DirectX::XMMatrixIdentity());
            return identity;
        }
        return m_finalBoneMatrices[static_cast<size_t>(boneIndex)];
    }

} // namespace Engine
