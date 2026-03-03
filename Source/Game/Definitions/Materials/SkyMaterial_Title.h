#pragma once

#include <memory>
#include <string.h>

#include "Engine/Materials/MaterialBuildContext.h"


namespace Engine {
    class Material;
    class AssetManager;

    struct MaterialBuildContext;
}

namespace Game {


    /**
     * @brief Skyドーム用（とりあえず既存 DefaultVS/DefaultPS を使う）
     * - テクスチャ：Assets/Textures/Sky.png を想定
     */
    class SkyMaterial_Title final {
    public:
        static constexpr const char* kName = "SkyMaterial";

        static std::shared_ptr<Engine::Material> Create(const Engine::MaterialBuildContext& ctx);
    };

} // namespace Game
