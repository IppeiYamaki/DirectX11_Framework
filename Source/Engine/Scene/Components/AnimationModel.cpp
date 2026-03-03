#include "AnimationModel.h"

#include "Engine/Core/Logger.h"
#include "Engine/Resources/AssetManager.h"
#include "Engine/Resources/SkinnedModelLoader.h"

#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Graphics/SkinnedModel.h"

#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"

namespace Engine {

    AnimationModel::AnimationModel(AssetManager* assets, ID3D11Device* device, RenderSystem* renderSystem)
        : m_assets(assets)
        , m_device(device)
        , m_renderSystem(renderSystem) {
    }

    void AnimationModel::OnStart() {
		// 初期状態ではモデルはロードされていないため、特に処理は行わない
  
    }

    bool AnimationModel::LoadModel(const std::wstring& path) {
        if (!m_assets || !m_device) {
            Logger::Error("AnimationModel::LoadModel failed: assets/device null.");
            return false;
        }

        m_model = LoadSkinnedModel(m_device, *m_assets, path);
        if (!m_model) {
            Logger::Error("AnimationModel::LoadModel failed: LoadSkinnedModel returned null.");
            return false;
        }

		// モデルのクリップ名を取得して、初期状態のブレンド対象をセット
        const auto names = m_model->GetClipNames();
        if (!names.empty()) {
            m_clipA = names[0];
            m_clipB = names[0];
        }
        else {
            m_clipA.clear();
            m_clipB.clear();
        }

        m_boneMatrices.clear();
        m_frameAccA = 0.0f;
        m_frameAccB = 0.0f;
        m_frameA = 0;
        m_frameB = 0;

        RebuildBones();
        return true;
    }

    void AnimationModel::SetBlend(const std::string& clipA, const std::string& clipB, float blendRate) {
        m_clipA = clipA;
        m_clipB = clipB;
        if (blendRate < 0.0f) blendRate = 0.0f;
        if (blendRate > 1.0f) blendRate = 1.0f;
        m_blendRate = blendRate;
    }

    void AnimationModel::SetFrames(int frameA, int frameB) {
        m_frameA = frameA;
        m_frameB = frameB;
    }

    void AnimationModel::SetPlaybackFps(float fps) {
        if (fps <= 1.0f) fps = 1.0f;
        m_fps = fps;
    }

    void AnimationModel::Update(float deltaTime) {
        if (!m_model) return;

        // フレームの進行
        m_frameAccA += deltaTime * m_fps;
        m_frameAccB += deltaTime * m_fps;
        m_frameA = static_cast<int>(m_frameAccA);
        m_frameB = static_cast<int>(m_frameAccB);

        RebuildBones();
    }

    void AnimationModel::RebuildBones() {
        if (!m_model) return;
        if (m_clipA.empty()) return;

        const bool ok = m_model->EvaluateBlendFrames(
            m_clipA, m_frameA,
            (m_clipB.empty() ? m_clipA : m_clipB), m_frameB,
            m_blendRate,
            m_boneMatrices
        );

        if (!ok) {
            // 失敗した場合はボーン行列を初期化
            m_boneMatrices.resize(m_model->GetBones().size());
            for (auto& m : m_boneMatrices) {
                DirectX::XMStoreFloat4x4(&m, DirectX::XMMatrixIdentity());
            }
        }
    }

    void AnimationModel::Draw() {
        if (!m_renderSystem) return;
        if (!m_model || !m_model->IsValid()) return;

        auto* owner = GetOwner();
        if (!owner) return;

        auto* tr = owner->GetComponent<Engine::Transform>();
        if (!tr) return;

        auto& mesh = m_model->GetMesh();
        const auto& subsets = m_model->GetSubsets();

        for (const auto& s : subsets) {
            if (!s.m_material || !s.m_material->IsInitialized()) continue;

            RenderItem item{};
            item.m_mesh = &mesh;
            item.m_material = s.m_material.get();
            item.m_world = tr->GetWorldMatrix();
            item.m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

			item.m_layer = RenderLayer::Opaque; // layerを固定しているが、将来的にはマテリアル側で指定できるようにするかも
			item.m_orderInLayer = 0;            // orderInLayerも固定しているが、将来的にはマテリアル側で指定できるようにするかも
			item.m_stateFlags = 0;              // 描画ステートフラグは今のところ使用していないが、将来的にはマテリアル側で指定できるようにするかも



            m_renderSystem->AddRenderItem(item);
        }
    }

    void AnimationModel::OnDestroy() {
        m_model.reset();
        m_boneMatrices.clear();
    }

    SkinnedModel* AnimationModel::GetModel() const {
        return m_model.get();
    }

} // namespace Engine
