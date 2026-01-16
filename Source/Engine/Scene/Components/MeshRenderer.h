#pragma once

#include <d3d11.h>
#include <memory>

#include "Engine/Scene/Component.h"
#include "Engine/Graphics/Mesh.h"
#include "Engine/Graphics/MeshFactory.h"
#include "Engine/Graphics/RenderLayer.h"

namespace Engine {

    class RenderSystem;
    class Material;

    class MeshRenderer final : public Component {
    public:
        MeshRenderer(ID3D11Device* device, RenderSystem* renderSystem);
        ~MeshRenderer() override = default;

        void OnStart() override;
        void Draw() override;
        void OnDestroy() override;

        void SetMeshType(MeshType type);
        void SetMeshDesc(const MeshCreateDesc& desc);

        void SetMaterial(const std::shared_ptr<Material>& material);
        Material* GetMaterial() const;

        // ★Unity風：描画レイヤー
        void SetRenderLayer(RenderLayer layer);
        RenderLayer GetRenderLayer() const;

        void SetOrderInLayer(int orderInLayer);
        int GetOrderInLayer() const;

        void SetRenderStateFlags(std::uint32_t flags);
        std::uint32_t GetRenderStateFlags() const;

    private:
        void CreateMeshIfNeeded();

    private:
        ID3D11Device* m_device = nullptr;       // borrowed
        RenderSystem* m_renderSystem = nullptr; // borrowed

        std::shared_ptr<Material> m_material;

        MeshCreateDesc m_meshDesc{};
        bool m_isMeshDirty = true;
        bool m_isMeshReady = false;

        Mesh m_mesh; // owning

        // ★描画順/ステート
        RenderLayer  m_renderLayer = RenderLayer::Opaque;
        int          m_orderInLayer = 0;
        std::uint32_t m_stateFlags = kRenderStateNone;
    };

} // namespace Engine
