#pragma once

#include <cmath>
#include <algorithm>

namespace Engine::Easing {

    // 0..1 に収めたい時用（必要なら使う）
    inline float Clamp01(float t) {
        return std::clamp(t, 0.0f, 1.0f);
    }

    inline constexpr float kPi = 3.14159265358979323846f;

    //=================================================================
    // Sine
    //=================================================================
    inline float EaseInSine(float x) {
        return 1.0f - std::cos((x * kPi) / 2.0f);
    }
    inline float EaseOutSine(float x) {
        return std::sin((x * kPi) / 2.0f);
    }
    inline float EaseInOutSine(float x) {
        return -(std::cos(kPi * x) - 1.0f) / 2.0f;
    }

    //=================================================================
    // Quad
    //=================================================================
    inline float EaseInQuad(float x) {
        return x * x;
    }
    inline float EaseOutQuad(float x) {
        return 1.0f - (1.0f - x) * (1.0f - x);
    }
    inline float EaseInOutQuad(float x) {
        return x < 0.5f
            ? 2.0f * x * x
            : 1.0f - std::pow(-2.0f * x + 2.0f, 2.0f) / 2.0f;
    }

    //=================================================================
    // Cubic
    //=================================================================
    inline float EaseInCubic(float x) {
        return x * x * x;
    }
    inline float EaseOutCubic(float x) {
        return 1.0f - std::pow(1.0f - x, 3.0f);
    }
    inline float EaseInOutCubic(float x) {
        return x < 0.5f
            ? 4.0f * x * x * x
            : 1.0f - std::pow(-2.0f * x + 2.0f, 3.0f) / 2.0f;
    }

    //=================================================================
    // Quart
    //=================================================================
    inline float EaseInQuart(float x) {
        return x * x * x * x;
    }
    inline float EaseOutQuart(float x) {
        return 1.0f - std::pow(1.0f - x, 4.0f);
    }
    inline float EaseInOutQuart(float x) {
        return x < 0.5f
            ? 8.0f * x * x * x * x
            : 1.0f - std::pow(-2.0f * x + 2.0f, 4.0f) / 2.0f;
    }

    //=================================================================
    // Quint
    //=================================================================
    inline float EaseInQuint(float x) {
        return x * x * x * x * x;
    }
    inline float EaseOutQuint(float x) {
        return 1.0f - std::pow(1.0f - x, 5.0f);
    }
    inline float EaseInOutQuint(float x) {
        return x < 0.5f
            ? 16.0f * x * x * x * x * x
            : 1.0f - std::pow(-2.0f * x + 2.0f, 5.0f) / 2.0f;
    }

    //=================================================================
    // Expo
    //=================================================================
    inline float EaseInExpo(float x) {
        return x == 0.0f ? 0.0f : std::pow(2.0f, 10.0f * x - 10.0f);
    }
    inline float EaseOutExpo(float x) {
        return x == 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * x);
    }
    inline float EaseInOutExpo(float x) {
        if (x == 0.0f) return 0.0f;
        if (x == 1.0f) return 1.0f;
        return x < 0.5f
            ? std::pow(2.0f, 20.0f * x - 10.0f) / 2.0f
            : (2.0f - std::pow(2.0f, -20.0f * x + 10.0f)) / 2.0f;
    }

    //=================================================================
    // Circ
    //=================================================================
    inline float EaseInCirc(float x) {
        return 1.0f - std::sqrt(1.0f - x * x);
    }
    inline float EaseOutCirc(float x) {
        return std::sqrt(1.0f - (x - 1.0f) * (x - 1.0f));
    }
    inline float EaseInOutCirc(float x) {
        return x < 0.5f
            ? (1.0f - std::sqrt(1.0f - (2.0f * x) * (2.0f * x))) / 2.0f
            : (std::sqrt(1.0f - (-2.0f * x + 2.0f) * (-2.0f * x + 2.0f)) + 1.0f) / 2.0f;
    }

    //=================================================================
    // Back
    //=================================================================
    inline float EaseInBack(float x) {
        constexpr float c1 = 1.70158f;
        constexpr float c3 = c1 + 1.0f;
        return c3 * x * x * x - c1 * x * x;
    }
    inline float EaseOutBack(float x) {
        constexpr float c1 = 1.70158f;
        constexpr float c3 = c1 + 1.0f;
        return 1.0f + c3 * std::pow(x - 1.0f, 3.0f) + c1 * std::pow(x - 1.0f, 2.0f);
    }
    inline float EaseInOutBack(float x) {
        constexpr float c1 = 1.70158f;
        constexpr float c2 = c1 * 1.525f;
        return x < 0.5f
            ? (std::pow(2.0f * x, 2.0f) * ((c2 + 1.0f) * 2.0f * x - c2)) / 2.0f
            : (std::pow(2.0f * x - 2.0f, 2.0f) * ((c2 + 1.0f) * (x * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
    }

    //=================================================================
    // Elastic
    //=================================================================
    inline float EaseInElastic(float x) {
        const float c4 = (2.0f * kPi) / 3.0f;
        if (x == 0.0f) return 0.0f;
        if (x == 1.0f) return 1.0f;
        return -std::pow(2.0f, 10.0f * x - 10.0f) * std::sin((x * 10.0f - 10.75f) * c4);
    }
    inline float EaseOutElastic(float x) {
        const float c4 = (2.0f * kPi) / 3.0f;
        if (x == 0.0f) return 0.0f;
        if (x == 1.0f) return 1.0f;
        return std::pow(2.0f, -10.0f * x) * std::sin((x * 10.0f - 0.75f) * c4) + 1.0f;
    }
    inline float EaseInOutElastic(float x) {
        const float c5 = (2.0f * kPi) / 4.5f;
        if (x == 0.0f) return 0.0f;
        if (x == 1.0f) return 1.0f;
        return x < 0.5f
            ? -(std::pow(2.0f, 20.0f * x - 10.0f) * std::sin((20.0f * x - 11.125f) * c5)) / 2.0f
            : (std::pow(2.0f, -20.0f * x + 10.0f) * std::sin((20.0f * x - 11.125f) * c5)) / 2.0f + 1.0f;
    }

    //=================================================================
    // Bounce
    //=================================================================
    inline float EaseOutBounce(float x) {
        constexpr float n1 = 7.5625f;
        constexpr float d1 = 2.75f;

        if (x < 1.0f / d1) return n1 * x * x;
        if (x < 2.0f / d1) { x -= 1.5f / d1; return n1 * x * x + 0.75f; }
        if (x < 2.5f / d1) { x -= 2.25f / d1; return n1 * x * x + 0.9375f; }
        x -= 2.625f / d1; return n1 * x * x + 0.984375f;
    }
    inline float EaseInBounce(float x) {
        return 1.0f - EaseOutBounce(1.0f - x);
    }
    inline float EaseInOutBounce(float x) {
        return x < 0.5f
            ? (1.0f - EaseOutBounce(1.0f - 2.0f * x)) / 2.0f
            : (1.0f + EaseOutBounce(2.0f * x - 1.0f)) / 2.0f;
    }

} // namespace Engine::Easing
