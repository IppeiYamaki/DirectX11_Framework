#include "StaticModelRenderer.h"

#include "Engine/Core/Logger.h"

#include "Engine/Resources/AssetManager.h"
#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Graphics/Model.h"

#include "Engine/Scene/Entity.h"
#include "Engine/Scene/Components/Transform.h"

namespace Engine {

    StaticModelRenderer::StaticModelRenderer(AssetManager* assets, ID3D11Device* device, RenderSystem* renderSystem)
        : m_assets(assets)
        , m_device(device)
        , m_renderSystem(renderSystem) {
    }

    void StaticModelRenderer::OnStart() {
        // 何もしない（LoadModel/SetModelされたら描画される）
    }

    void StaticModelRenderer::Draw() {
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

			item.m_layer = RenderLayer::Opaque; // TODO: マテリアルから取る
			item.m_orderInLayer = 0;            // TODO: マテリアルから取る
			item.m_stateFlags = 0;              // TODO: マテリアルから取る

            m_renderSystem->AddRenderItem(item);
        }
    }

    void StaticModelRenderer::OnDestroy() {
        m_model.reset();
    }

    bool StaticModelRenderer::LoadModel(const std::wstring& path) {
        if (!m_assets) {
            Logger::Error("StaticModelRenderer::LoadModel failed: assets is null.");
            return false;
        }

        auto model = m_assets->LoadModel(path);
        if (!model) {
            Logger::Error("StaticModelRenderer::LoadModel failed: AssetManager::LoadModel returned null.");
            return false;
        }

        m_model = std::move(model);
        return true;
    }

    void StaticModelRenderer::SetModel(const std::shared_ptr<Model>& model) {
        m_model = model;
    }

    Model* StaticModelRenderer::GetModel() const {
        return m_model.get();
    }

} // namespace Engine
