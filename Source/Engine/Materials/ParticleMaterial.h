#pragma once

#include <memory>

#include "Engine/Materials/MaterialBuildContext.h"

namespace Engine {

    class Material;

    /**
     * @brief パーティクル用マテリアル
     * - Billboard描画
     * - 加算合成またはアルファブレンド
     * - テクスチャなし（色のみ）
     */
    class ParticleMaterial final {
    public:
        static constexpr const char* kName = "ParticleMaterial";

        static std::shared_ptr<Engine::Material> Create(const Engine::MaterialBuildContext& ctx);
    };

} // namespace Engine
