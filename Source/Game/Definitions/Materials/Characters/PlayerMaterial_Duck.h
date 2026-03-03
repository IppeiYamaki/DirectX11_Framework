#pragma once

#include <memory>

#include "Engine/Materials/MaterialBuildContext.h"

namespace Engine { class Material; }

namespace Game {

    /**
     * @brief プレイヤー用アヒルマテリアル（Unityの .mat 相当をコードで定義）
     */
    class PlayerMaterial_Duck final {
    public:
        static constexpr const char* kName = "PlayerMaterial_Duck";

        static std::shared_ptr<Engine::Material> Create(const Engine::MaterialBuildContext& ctx);
    };

} // namespace Game
