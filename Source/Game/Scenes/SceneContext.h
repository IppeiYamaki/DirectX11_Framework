#pragma once

#include <memory>
#include <d3d11.h> // ID3D11Device
#include <utility> 

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
     * @brief Sceneが使う “最低限のサービス” を束ねたもの
     * - Sceneは Engine の巨大クラスに依存しない（依存を薄くする）
     */
    struct SceneContext final {
        // Engine側のサービス（最小）
        Engine::Application* m_app = nullptr;
        Engine::World* m_world = nullptr;
        Engine::RenderSystem* m_renderSystem = nullptr;

        // 薄い依存：生のD3Dデバイス
        ID3D11Device* m_device = nullptr; // borrowed

        // 共有リソース（GameMainが用意してSceneに渡す）
        Engine::AssetManager* m_assets = nullptr;


        // 共有Material（Scene内でMeshRendererに渡す）
        std::shared_ptr<Engine::Material> m_sharedMaterial;

		MaterialLibrary* m_materials = nullptr;

        template<class TPrefab, class... Args>
        /// @brief Prefabを生成する（引数は Prefab::SpawnDesc のコンストラクタ引数に転送される）
		/// @param pos 生成位置
		/// @param scale 生成スケール
		/// @param rot 生成時回転
        Engine::Entity* Spawn(Args&&... args) {
            using Desc = typename TPrefab::SpawnDesc;
            return TPrefab::Spawn(*this, Desc{ std::forward<Args>(args)... });
        }
    };

} // namespace Game
