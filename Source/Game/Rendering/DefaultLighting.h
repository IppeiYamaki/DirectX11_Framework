#pragma once

namespace Engine { class RenderSystem; }

namespace Game {

    /// @brief ゲーム側のデフォルトライトを適用する
    void ApplyDefaultLighting(Engine::RenderSystem& renderSystem);

} // namespace Game
