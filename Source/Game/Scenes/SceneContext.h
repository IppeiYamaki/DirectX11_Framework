#pragma once

#include <memory>
#include <utility>
#include <d3d11.h> // ID3D11Device

namespace Engine {
    class Application;
    class Entity;
    class World;
    class RenderSystem;
    class AssetManager;
    class Material;
}

namespace Game {

    class MaterialLibrary;

    /**
     * @brief Sceneが使う最低限のサービスをまとめたコンテキスト
     * - Sceneは Engine の巨大クラスに直接依存しない（依存を最小化）
     */
    struct SceneContext final {
        // Engine側サービス（最小）
        Engine::Application* m_app = nullptr;
        Engine::World* m_world = nullptr;
        Engine::RenderSystem* m_renderSystem = nullptr;

        // 薄い依存：生のD3Dデバイス（借用）
        ID3D11Device* m_device = nullptr;

        // 共有リソース（GameMainが用意してSceneに渡す）
        Engine::AssetManager* m_assets = nullptr;

        // 共有Material（Scene内で直接使いたい場合）
        std::shared_ptr<Engine::Material> m_sharedMaterial;

        // Material資産管理
        MaterialLibrary* m_materials = nullptr;

        /// @brief Prefabを生成する（引数は Prefab::SpawnDesc のコンストラクタ引数に転送される）
        /// @param pos 生成位置
        /// @param scale 生成スケール
        /// @param rot 生成時回転
        template<class TPrefab, class... Args>
        Engine::Entity* Spawn(Args&&... args) {
            using Desc = typename TPrefab::SpawnDesc;
            return TPrefab::Spawn(*this, Desc{ std::forward<Args>(args)... });
        }
    };

} // namespace Game
