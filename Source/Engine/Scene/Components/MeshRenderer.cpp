#include "MeshRenderer.h"

#include "Engine/Core/Logger.h"

#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"

#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Graphics/Material.h"

namespace Engine {

    MeshRenderer::MeshRenderer(ID3D11Device* device, RenderSystem* renderSystem)
        : m_device(device)
        , m_renderSystem(renderSystem) {
    }

    void MeshRenderer::OnStart() {
        CreateMeshIfNeeded();
    }

    void MeshRenderer::Draw() {
        if (!m_renderSystem) return;
        if (!m_material) return;

        CreateMeshIfNeeded();
        if (!m_isMeshReady || !m_mesh.IsValid()) return;

        auto* owner = GetOwner();
        if (!owner) return;

        auto* tr = owner->GetComponent<Engine::Transform>();
        if (!tr) return;

        RenderItem item{};
        item.m_mesh = &m_mesh;
        item.m_material = m_material.get();
        item.m_world = tr->GetWorldMatrix();
        item.m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        // 
        item.m_layer = m_renderLayer;
        item.m_orderInLayer = m_orderInLayer;
        item.m_stateFlags = m_stateFlags;

        m_renderSystem->AddRenderItem(item);
    }

    void MeshRenderer::OnDestroy() {
        m_mesh.Destroy();
        m_isMeshReady = false;
    }

    void MeshRenderer::SetMeshType(MeshType type) {
        m_meshDesc = MeshCreateDesc{};
        m_meshDesc.m_type = type;
        m_isMeshDirty = true;
    }

    void MeshRenderer::SetMeshDesc(const MeshCreateDesc& desc) {
        m_meshDesc = desc;
        m_isMeshDirty = true;
    }

    void MeshRenderer::SetMaterial(const std::shared_ptr<Material>& material) {
        m_material = material;
    }

    Material* MeshRenderer::GetMaterial() const {
        return m_material.get();
    }

    const Mesh* MeshRenderer::GetMesh() const {
        return &m_mesh;
    }

    bool MeshRenderer::IsMeshReady() const {
        return m_isMeshReady && m_mesh.IsValid();
    }

    void MeshRenderer::SetRenderLayer(RenderLayer layer) {
        m_renderLayer = layer;
    }

    RenderLayer MeshRenderer::GetRenderLayer() const {
        return m_renderLayer;
    }

    void MeshRenderer::SetOrderInLayer(int orderInLayer) {
        m_orderInLayer = orderInLayer;
    }

    int MeshRenderer::GetOrderInLayer() const {
        return m_orderInLayer;
    }

    void MeshRenderer::SetRenderStateFlags(std::uint32_t flags) {
        m_stateFlags = flags;
    }

    std::uint32_t MeshRenderer::GetRenderStateFlags() const {
        return m_stateFlags;
    }

    void MeshRenderer::CreateMeshIfNeeded() {
        if (m_isMeshReady && !m_isMeshDirty) return;
        if (!m_device) return;

        if (m_mesh.IsValid()) {
            m_mesh.Destroy();
        }

        if (!MeshFactory::Create(m_device, m_mesh, m_meshDesc)) {
            Logger::Error("MeshRenderer: MeshFactory::Create failed.");
            m_isMeshReady = false;
            return;
        }

        m_isMeshReady = true;
        m_isMeshDirty = false;
    }

} // namespace Engine
