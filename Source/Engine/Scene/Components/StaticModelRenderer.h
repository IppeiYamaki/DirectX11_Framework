#pragma once

#include <d3d11.h>
#include <memory>
#include <string>

#include "Engine/Scene/Component.h"

namespace Engine {
    class AssetManager;
    class RenderSystem;
    class Model;

    /**
     * @brief OBJモデル（Static）を描画するComponent
     * - Drawで ModelのSubset分だけ RenderItem を積む
     */
    class StaticModelRenderer final : public Component {
    public:
        StaticModelRenderer(AssetManager* assets, ID3D11Device* device, RenderSystem* renderSystem);
        ~StaticModelRenderer() override = default;

        void OnStart() override;
        void Draw() override;
        void OnDestroy() override;

        bool LoadModel(const std::wstring& path);
        void SetModel(const std::shared_ptr<Model>& model);

        Model* GetModel() const;

    private:
        AssetManager* m_assets = nullptr;         // borrowed
        ID3D11Device* m_device = nullptr;         // borrowed（今は未使用だが将来拡張用）
        RenderSystem* m_renderSystem = nullptr;   // borrowed

        std::shared_ptr<Model> m_model;
    };

} // namespace Engine
