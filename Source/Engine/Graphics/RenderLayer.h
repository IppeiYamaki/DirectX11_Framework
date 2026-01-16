#pragma once

#include <cstdint>

namespace Engine {

    /**
     * @brief Unityの「描画レイヤー」っぽいもの（描画順）
     * 小さいほど先に描く（背景→通常→透明→UI）
     */
    enum class RenderLayer : std::uint8_t {
        Background = 0,
        Opaque = 100,
        Transparent = 200,
        Overlay = 250,
    };

    /**
     * @brief RenderItem に付ける描画ステート指定（最低限）
     */
    enum RenderStateFlags : std::uint32_t {
        kRenderStateNone = 0,

        // Depth
        kRenderStateDepthWriteOff = 1u << 0, // 深度テストはするが書き込みしない（Skyなど）
        kRenderStateDepthTestOff = 1u << 1, // 深度テスト自体を切る（UIなど）

        // Rasterizer
        kRenderStateCullFront = 1u << 2, // Skyドーム内側表示向け
        kRenderStateCullNone = 1u << 3, // 両面表示

        // Blend（将来の透明用。今は用意だけ）
        kRenderStateBlendAlpha = 1u << 4,
    };

} // namespace Engine
