#pragma once

#include <memory>

#include "Materials/MaterialBuildContext.h"

namespace Engine { class Material; }

namespace Game {

    /**
     * @brief サンプル用Cubeマテリアル（Unityの .mat 相当をコードで定義）
     */
    class SampleCubeMaterial final {
    public:
        static constexpr const char* kName = "SampleCubeMaterial";

        static std::shared_ptr<Engine::Material> Create(const MaterialBuildContext& ctx);
    };

} // namespace Game
