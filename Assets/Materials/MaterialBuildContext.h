#pragma once
#include <d3d11.h>

namespace Engine { class AssetManager; }

namespace Game {

    // Material‚Ì¶¬‚É•K—v‚ÈÅ’áŒÀ
    struct MaterialBuildContext final {
        ID3D11Device* m_device = nullptr;        // borrowed
        Engine::AssetManager* m_assets = nullptr; // borrowed
    };

} // namespace Game
