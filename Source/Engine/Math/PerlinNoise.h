/// @file   PerlinNoise.h
/// @brief  Perlin Noise implementation for terrain generation
/// @note   Based on Ken Perlin's improved noise algorithm
#pragma once

#include <cstdint>
#include <vector>

namespace Engine {

    /// @brief Perlin Noise generator for natural terrain patterns
    /// @note  Implements improved Perlin noise with optional fractal (multi-octave) support
    class PerlinNoise final {
    public:
        /// @brief Constructor with optional seed
        /// @param seed Random seed for permutation table (default = 0)
        explicit PerlinNoise(std::uint32_t seed = 0);

        /// @brief Generate 2D Perlin noise value
        /// @param x X coordinate
        /// @param y Y coordinate (used as Z in terrain context)
        /// @return Noise value in range [0.0, 1.0]
        double noise(double x, double y) const;

        /// @brief Generate fractal Perlin noise with multiple octaves
        /// @param x X coordinate
        /// @param y Y coordinate
        /// @param octaves Number of noise layers to combine (default = 4)
        /// @param persistence Amplitude decay per octave (default = 0.5)
        /// @param lacunarity Frequency increase per octave (default = 2.0)
        /// @return Fractal noise value in range [0.0, 1.0]
        double fractal(double x, double y, int octaves = 4, 
                      double persistence = 0.5, double lacunarity = 2.0) const;

    private:
        /// @brief Fade function for smooth interpolation (6t^5 - 15t^4 + 10t^3)
        static double fade(double t);

        /// @brief Linear interpolation
        static double lerp(double t, double a, double b);

        /// @brief Gradient calculation
        static double grad(int hash, double x, double y);

        /// @brief Permutation table (256 values duplicated to 512)
        std::vector<int> m_permutation;
    };

} // namespace Engine
