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
     * @brief 水面用マテリアル
     * - 平面反射テクスチャを合成
     * - 波による歪みエフェクト
     * - αブレンド対応
     */
    class WaterMaterial final {
    public:
        static constexpr const char* kName = "WaterMaterial";

        static std::shared_ptr<Engine::Material> Create(const Engine::MaterialBuildContext& ctx);
    };

} // namespace Game
