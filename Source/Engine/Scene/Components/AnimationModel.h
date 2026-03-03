#pragma once

#include <d3d11.h>
#include <memory>
#include <string>
#include <vector>

#include <DirectXMath.h> // XMFLOAT4X4

#include "Engine/Scene/Component.h"

namespace Engine {

    class AssetManager;
    class RenderSystem;
    class SkinnedModel;

    /**
     * @brief スキンアニメモデル描画コンポーネント（GPUスキニング）
     * - Update: 骨行列の更新（フレーム進行）
     * - Draw  : Subsetごとに RenderItem を積む（b5に骨行列を渡す）
     */
    class AnimationModel final : public Component {
    public:
        AnimationModel(AssetManager* assets, ID3D11Device* device, RenderSystem* renderSystem);
        ~AnimationModel() override = default;

        void OnStart() override;
        void Update(float deltaTime) override;
        void Draw() override;
        void OnDestroy() override;

        bool LoadModel(const std::wstring& path);

        void SetBlend(const std::string& clipA, const std::string& clipB, float blendRate);
        void SetFrames(int frameA, int frameB);
        void SetPlaybackFps(float fps);

        SkinnedModel* GetModel() const;

    private:
        void RebuildBones();

    private:
        AssetManager* m_assets = nullptr;       // borrowed
        ID3D11Device* m_device = nullptr;       // borrowed
        RenderSystem* m_renderSystem = nullptr; // borrowed

        std::shared_ptr<SkinnedModel> m_model;

        // 変数として宣言
        std::vector<DirectX::XMFLOAT4X4> m_boneMatrices;

        std::string m_clipA{};
        std::string m_clipB{};
        float m_blendRate = 0.0f;

        float m_fps = 60.0f;
        float m_frameAccA = 0.0f;
        float m_frameAccB = 0.0f;
        int m_frameA = 0;
        int m_frameB = 0;
    };

} // namespace Engine
