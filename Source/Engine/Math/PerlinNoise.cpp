/// @file   PerlinNoise.cpp
/// @brief  Perlin Noise implementation
#include "PerlinNoise.h"

#include <algorithm>
#include <numeric>
#include <random>
#include <cmath>

namespace Engine {

    PerlinNoise::PerlinNoise(std::uint32_t seed) {
        // Initialize permutation table with values 0-255
        m_permutation.resize(256);
        std::iota(m_permutation.begin(), m_permutation.end(), 0);

        // Shuffle using seed with std::mt19937 for reproducible results across platforms
        std::mt19937 engine(seed);
        std::shuffle(m_permutation.begin(), m_permutation.end(), engine);

        // Duplicate the permutation table for wrapping
        m_permutation.insert(m_permutation.end(), m_permutation.begin(), m_permutation.end());
    }

    double PerlinNoise::noise(double x, double y) const {
        // Find unit grid cell containing point
        const int X = static_cast<int>(std::floor(x)) & 255;
        const int Y = static_cast<int>(std::floor(y)) & 255;

        // Get relative position within cell
        x -= std::floor(x);
        y -= std::floor(y);

        // Compute fade curves
        const double u = fade(x);
        const double v = fade(y);

        // Hash coordinates of the 4 cube corners
        const int a  = m_permutation[X]     + Y;
        const int aa = m_permutation[a];
        const int ab = m_permutation[a + 1];
        const int b  = m_permutation[X + 1] + Y;
        const int ba = m_permutation[b];
        const int bb = m_permutation[b + 1];

        // Blend results from corners
        const double x1 = lerp(u, grad(m_permutation[aa], x,     y    ),
                                   grad(m_permutation[ba], x - 1, y    ));
        const double x2 = lerp(u, grad(m_permutation[ab], x,     y - 1),
                                   grad(m_permutation[bb], x - 1, y - 1));
        const double result = lerp(v, x1, x2);

        // Map from [-1, 1] to [0, 1]
        return (result + 1.0) * 0.5;
    }

    double PerlinNoise::fractal(double x, double y, int octaves, 
                                 double persistence, double lacunarity) const {
        double total = 0.0;
        double frequency = 1.0;
        double amplitude = 1.0;
        double maxValue = 0.0;  // Used for normalizing result to 0.0 - 1.0

        for (int i = 0; i < octaves; ++i) {
            total += noise(x * frequency, y * frequency) * amplitude;

            maxValue += amplitude;
            amplitude *= persistence;
            frequency *= lacunarity;
        }

        return total / maxValue;
    }

    double PerlinNoise::fade(double t) {
        // 6t^5 - 15t^4 + 10t^3
        return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
    }

    double PerlinNoise::lerp(double t, double a, double b) {
        return a + t * (b - a);
    }

    double PerlinNoise::grad(int hash, double x, double y) {
        // Convert low 4 bits of hash into 8 gradient directions
        const int h = hash & 7;
        const double u = (h < 4) ? x : y;
        const double v = (h < 4) ? y : x;
        return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
    }

} // namespace Engine
