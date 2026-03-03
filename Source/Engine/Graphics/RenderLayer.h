#pragma once

#include <cstdint>

namespace Engine {

    /// @brief RenderLayer
    /// @brief Background:   Background (Sky, etc.)
    /// @brief Opaque:       Opaque objects (normal geometry)
    /// @brief Transparent:  Transparent objects (particles, water, etc.)
    /// @brief Overlay:      Overlay (UI, etc.)
    /// @brief Debug:        Debug visualization (outlines, light icons, etc.) - always on top
    enum class RenderLayer : std::uint8_t {
        Background = 0,
        Opaque = 100,
        Transparent = 200,
        Overlay = 250,
        Debug = 255,
    };

    /// @brief RenderStateFlags
    enum RenderStateFlags : std::uint32_t {
        kRenderStateNone = 0,

        // Depth
        kRenderStateDepthWriteOff   = 1u << 0, // Depth test ON, write OFF (Sky, etc.)
        kRenderStateDepthTestOff    = 1u << 1, // Depth test OFF (UI, etc.)

        // Rasterizer
        kRenderStateCullFront       = 1u << 2, // Cull front for sky dome inside
        kRenderStateCullNone        = 1u << 3, // Double-sided rendering

        // Blend
        kRenderStateBlendAlpha      = 1u << 4, // Alpha blend
        kRenderStateBlendAdditive   = 1u << 5, // Additive blend (particles)
    };

} // namespace Engine
