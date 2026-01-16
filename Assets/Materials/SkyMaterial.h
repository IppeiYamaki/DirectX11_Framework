#pragma once

#include <memory>
#include <string.h>

#include "Materials/MaterialBuildContext.h"


namespace Engine {
    class Material;
    class AssetManager;
}

namespace Game {

    struct MaterialBuildContext;

    /**
     * @brief Skyドーム用（とりあえず既存 DefaultVS/DefaultPS を使う）
     * - テクスチャ：Assets/Textures/Sky.png を想定
     */
    class SkyMaterial final {
    public:
        static constexpr const char* kName = "SkyMaterial";

        static std::shared_ptr<Engine::Material> Create(const MaterialBuildContext& ctx);
    };

} // namespace Game
