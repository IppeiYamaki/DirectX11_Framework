#pragma once

#include <d3d11.h>
#include <memory>

#include "Engine/Scene/Component.h"
#include "Engine/Graphics/Mesh.h"
#include "Engine/Graphics/RenderLayer.h"

namespace Engine {
    class RenderSystem;
    class Material;
}

namespace Game {

    class FireflyParticleComponent;

    /**
     * @brief パーティクルレンダラーコンポーネント
     * - Billboard描画（常にカメラ正面を向く）
     * - 加算合成またはアルファブレンド対応
     * - FireflyParticleComponentと連携して色と輝度を反映
     */
    class ParticleRendererComponent final : public Engine::Component {
    public:
        /// @brief コンストラクタ
        /// @param device D3D11デバイス
        /// @param renderSystem RenderSystem参照
        ParticleRendererComponent(ID3D11Device* device, Engine::RenderSystem* renderSystem);
        ~ParticleRendererComponent() override = default;

        void OnStart() override;
        void Draw() override;
        void OnDestroy() override;

        /// @brief マテリアルを設定
        void SetMaterial(const std::shared_ptr<Engine::Material>& material);

        /// @brief 描画レイヤーを設定
        void SetRenderLayer(Engine::RenderLayer layer);

        /// @brief レイヤー内描画順を設定
        void SetOrderInLayer(int orderInLayer);

        /// @brief 描画ステートフラグを設定
        void SetRenderStateFlags(std::uint32_t flags);

    private:
        /// @brief Billboardメッシュを作成（単純な四角形）
        void CreateBillboardMesh();

    private:
        ID3D11Device* m_device = nullptr;
        Engine::RenderSystem* m_renderSystem = nullptr;

        std::shared_ptr<Engine::Material> m_material;
        Engine::Mesh m_mesh;
        bool m_isMeshReady = false;

        Engine::RenderLayer m_renderLayer = Engine::RenderLayer::Transparent;
        int m_orderInLayer = 0;
        std::uint32_t m_stateFlags = Engine::kRenderStateNone;

        FireflyParticleComponent* m_particleComponent = nullptr;
    };

} // namespace Game
