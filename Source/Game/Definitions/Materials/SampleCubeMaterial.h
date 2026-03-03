#pragma once

#include <memory>

#include "Engine/Materials/MaterialBuildContext.h"

namespace Engine { class Material; }

namespace Game {

    /**
     * @brief サンプル用Cubeマテリアル（Unityの .mat 相当をコードで定義）
     */
    class SampleCubeMaterial final {
    public:
        static constexpr const char* kName = "SampleCubeMaterial";

        static std::shared_ptr<Engine::Material> Create(const Engine::MaterialBuildContext& ctx);
    };

} // namespace Game
