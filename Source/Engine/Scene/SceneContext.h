#pragma once

#include <memory>
#include <utility>
#include <d3d11.h> // ID3D11Device

namespace Engine {
    class Application;
    class Entity;
    class Scene;
    class RenderSystem;
    class AssetManager;
    class Material;
}

namespace Game {

    class MaterialLibrary;

    /**
     * @brief Scene利用に最低限のサービスをまとめたコンテキスト
     * - SceneはEngineの具象クラスに直接依存しない（依存を最小限に）
     */
    struct SceneContext final {
        // Engine側サービス（最小）
        Engine::Application* m_app = nullptr;
        Engine::Scene* m_scene = nullptr;
        Engine::RenderSystem* m_renderSystem = nullptr;

        // 一部依存：借用D3Dデバイス（借用）
        ID3D11Device* m_device = nullptr;

        // 共有リソース（GameMainが用意してSceneに渡す）
        Engine::AssetManager* m_assets = nullptr;

        // 共有Material（Scene側で直接使いたい場合）
        std::shared_ptr<Engine::Material> m_sharedMaterial;

        // Material資産管理
        MaterialLibrary* m_materials = nullptr;

        /// @brief Prefabを生成する（引数は Prefab::SpawnDesc のコンストラクタ引数に転送される）
        /// @param pos 生成位置
        /// @param scale 生成スケール
        /// @param rot 生成回転
        template<class TPrefab, class... Args>
        Engine::Entity* Spawn(Args&&... args) {
            using Desc = typename TPrefab::SpawnDesc;
            return TPrefab::Spawn(*this, Desc{ std::forward<Args>(args)... });
        }
    };

} // namespace Game
