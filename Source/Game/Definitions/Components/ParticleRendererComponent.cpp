#include "ParticleRendererComponent.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Graphics/Material.h"
#include "Engine/Graphics/MeshPrimitives.h"
#include "Game/Definitions/Components/FireflyParticleComponent.h"

#include <vector>

namespace Game {

    ParticleRendererComponent::ParticleRendererComponent(ID3D11Device* device, Engine::RenderSystem* renderSystem)
        : m_device(device)
        , m_renderSystem(renderSystem) {
    }

    void ParticleRendererComponent::OnStart() {
        CreateBillboardMesh();

        // Get FireflyParticleComponent from the same GameObject
        auto* owner = GetOwner();
        if (owner) {
            m_particleComponent = owner->GetComponent<FireflyParticleComponent>();
        }
    }

    void ParticleRendererComponent::Draw() {
        if (!m_isMeshReady || !m_material || !m_renderSystem) {
            return;
        }

        auto* owner = GetOwner();
        if (!owner) return;

        auto* transform = owner->GetComponent<Engine::Transform>();
        if (!transform) return;

        // Get particle size and color from FireflyParticleComponent
        float size = 0.3f;
        Engine::Color color(1.0f, 1.0f, 0.8f, 1.0f);

        if (m_particleComponent) {
            size = m_particleComponent->GetSize();
            color = m_particleComponent->GetCurrentColor();
        }

        // Build world matrix with scale
        auto pos = transform->GetPosition();
        DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixScaling(size, size, size) *
                                         DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);

        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, worldMatrix);

        // Set material color
        if (m_material) {
            auto& params = m_material->GetParams();
            params.m_baseColor = Engine::Vector4(color.x, color.y, color.z, color.w);
        }

        // Create and submit render item
        Engine::RenderItem item;
        item.m_mesh = &m_mesh;
        item.m_material = m_material.get();
        item.m_world = world;
        item.m_layer = m_renderLayer;
        item.m_orderInLayer = m_orderInLayer;
        item.m_stateFlags = m_stateFlags;
        item.m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        m_renderSystem->AddRenderItem(item);
    }

    void ParticleRendererComponent::OnDestroy() {
        m_mesh.Destroy();
        m_isMeshReady = false;
    }

    void ParticleRendererComponent::SetMaterial(const std::shared_ptr<Engine::Material>& material) {
        m_material = material;
    }

    void ParticleRendererComponent::SetRenderLayer(Engine::RenderLayer layer) {
        m_renderLayer = layer;
    }

    void ParticleRendererComponent::SetOrderInLayer(int orderInLayer) {
        m_orderInLayer = orderInLayer;
    }

    void ParticleRendererComponent::SetRenderStateFlags(std::uint32_t flags) {
        m_stateFlags = flags;
    }

    void ParticleRendererComponent::CreateBillboardMesh() {
        if (!m_device) {
            Engine::Logger::Error("ParticleRendererComponent: Device is null");
            return;
        }

        // Create a simple quad for billboard
        // The vertex shader will expand this into a camera-facing billboard
        std::vector<Engine::VertexPosNormColorUv> vertices = {
            // Bottom-left
            { Engine::Vector3(0.0f, 0.0f, 0.0f), Engine::Vector3(0.0f, 1.0f, 0.0f), Engine::Vector4(1.0f, 1.0f, 1.0f, 1.0f), Engine::Vector2(0.0f, 1.0f) },
            // Bottom-right
            { Engine::Vector3(0.0f, 0.0f, 0.0f), Engine::Vector3(0.0f, 1.0f, 0.0f), Engine::Vector4(1.0f, 1.0f, 1.0f, 1.0f), Engine::Vector2(1.0f, 1.0f) },
            // Top-right
            { Engine::Vector3(0.0f, 0.0f, 0.0f), Engine::Vector3(0.0f, 1.0f, 0.0f), Engine::Vector4(1.0f, 1.0f, 1.0f, 1.0f), Engine::Vector2(1.0f, 0.0f) },
            // Top-left
            { Engine::Vector3(0.0f, 0.0f, 0.0f), Engine::Vector3(0.0f, 1.0f, 0.0f), Engine::Vector4(1.0f, 1.0f, 1.0f, 1.0f), Engine::Vector2(0.0f, 0.0f) },
        };

        std::vector<std::uint32_t> indices = {
            0, 1, 2,
            0, 2, 3
        };

        // Use Mesh::Create() to properly create mesh with vertex and index buffers
        if (!m_mesh.Create(
            m_device,
            vertices.data(),
            static_cast<std::uint32_t>(sizeof(Engine::VertexPosNormColorUv)),
            static_cast<std::uint32_t>(vertices.size()),
            indices.data(),
            static_cast<std::uint32_t>(indices.size())
        )) {
            Engine::Logger::Error("ParticleRendererComponent: Failed to create billboard mesh");
            return;
        }

        m_isMeshReady = true;

        Engine::Logger::Info("ParticleRendererComponent: Billboard mesh created");
    }

} // namespace Game
