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
     * @brief タイトル画像用Unlitマテリアル
     * - テクスチャ：Assets/Textures/UI/Title.png を使用
     * - ライト影響なし（Unlit）でそのままの色で表示
     * - αブレンド対応（透過PNG対応）
     */
    class TitleQuadMaterial final {
    public:
        static constexpr const char* kName = "TitleQuadMaterial";

        static std::shared_ptr<Engine::Material> Create(const Engine::MaterialBuildContext& ctx);
    };

} // namespace Game
