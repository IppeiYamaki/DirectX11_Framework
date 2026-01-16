#pragma once

#include <d3d11.h>
#include <memory>

#include "Engine/Scene/Component.h"
#include "Engine/Graphics/Mesh.h"
#include "Engine/Graphics/MeshFactory.h"

namespace Engine {

    class RenderSystem;
    class Material;

    /**
     * @brief Unity•— MeshRenderer
     * - MeshType / MeshCreateDesc ‚ğ•Û
     * - ‰‰ñ‚¾‚¯ Mesh ‚ğ¶¬
     * - –ˆƒtƒŒ[ƒ€ RenderItem ‚ğ RenderSystem ‚ÉÏ‚Ş
     */
    class MeshRenderer final : public Component {
    public:
        // šAddComponent ‚©‚ç“n‚·ˆø”‚Æˆê’v‚³‚¹‚é
        MeshRenderer(ID3D11Device* device, RenderSystem* renderSystem);
        ~MeshRenderer() override = default;

        void OnStart() override;
        void Draw() override;
        void OnDestroy() override;

        void SetMeshType(MeshType type);
        void SetMeshDesc(const MeshCreateDesc& desc);

        void SetMaterial(const std::shared_ptr<Material>& material);
        Material* GetMaterial() const;

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
    };

} // namespace Engine
