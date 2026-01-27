#pragma once

/// @file   MathConstants.h
/// @brief  数学定数とユーティリティ関数（C++20 constexpr対応）

#include <numbers>
#include <cmath>

namespace Engine::Math {

    //============================================================
    // 定数
    //============================================================

    /// @brief 円周率 π
    inline constexpr float kPi = std::numbers::pi_v<float>;

    /// @brief 2π
    inline constexpr float kTwoPi = kPi * 2.0f;

    /// @brief π/2
    inline constexpr float kHalfPi = kPi / 2.0f;

    /// @brief π/4
    inline constexpr float kQuarterPi = kPi / 4.0f;

    /// @brief ラジアン変換係数（度 → ラジアン）
    inline constexpr float kDegToRad = kPi / 180.0f;

    /// @brief 度変換係数（ラジアン → 度）
    inline constexpr float kRadToDeg = 180.0f / kPi;

    /// @brief 浮動小数点の許容誤差
    inline constexpr float kEpsilon = 1e-6f;

    /// @brief 自然対数の底 e
    inline constexpr float kE = std::numbers::e_v<float>;

    /// @brief √2
    inline constexpr float kSqrt2 = std::numbers::sqrt2_v<float>;

    /// @brief 1/√2
    inline constexpr float kInvSqrt2 = 1.0f / kSqrt2;

    //============================================================
    // 変換関数
    //============================================================

    /// @brief  度からラジアンへの変換
    /// @param  degrees 角度（度）
    /// @return 角度（ラジアン）
    [[nodiscard]] constexpr float ToRadians(float degrees) noexcept {
        return degrees * kDegToRad;
    }

    /// @brief  ラジアンから度への変換
    /// @param  radians 角度（ラジアン）
    /// @return 角度（度）
    [[nodiscard]] constexpr float ToDegrees(float radians) noexcept {
        return radians * kRadToDeg;
    }

    //============================================================
    // ユーティリティ関数
    //============================================================

    /// @brief  値を範囲内にクランプ
    /// @param  value 入力値
    /// @param  minVal 最小値
    /// @param  maxVal 最大値
    /// @return クランプされた値
    [[nodiscard]] constexpr float Clamp(float value, float minVal, float maxVal) noexcept {
        return (value < minVal) ? minVal : (value > maxVal) ? maxVal : value;
    }

    /// @brief  0.0〜1.0の範囲にクランプ
    /// @param  value 入力値
    /// @return クランプされた値
    [[nodiscard]] constexpr float Saturate(float value) noexcept {
        return Clamp(value, 0.0f, 1.0f);
    }

    /// @brief  線形補間
    /// @param  a 開始値
    /// @param  b 終了値
    /// @param  t 補間係数（0.0〜1.0）
    /// @return 補間結果
    [[nodiscard]] constexpr float Lerp(float a, float b, float t) noexcept {
        return a + (b - a) * t;
    }

    /// @brief  逆線形補間（aからbの間でvalueがどの位置かを返す）
    /// @param  a 開始値
    /// @param  b 終了値
    /// @param  value 入力値
    /// @return 補間係数（0.0〜1.0）
    [[nodiscard]] constexpr float InverseLerp(float a, float b, float value) noexcept {
        if (const float range = b - a; range != 0.0f) {
            return (value - a) / range;
        }
        return 0.0f;
    }

    /// @brief  符号を返す
    /// @param  value 入力値
    /// @return 正なら1.0f、負なら-1.0f、0なら0.0f
    [[nodiscard]] constexpr float Sign(float value) noexcept {
        return (value > 0.0f) ? 1.0f : (value < 0.0f) ? -1.0f : 0.0f;
    }

    /// @brief  ほぼゼロかどうかを判定
    /// @param  value 入力値
    /// @param  epsilon 許容誤差
    /// @return ほぼゼロならtrue
    [[nodiscard]] constexpr bool IsNearlyZero(float value, float epsilon = kEpsilon) noexcept {
        return (value > -epsilon) && (value < epsilon);
    }

    /// @brief  ほぼ等しいかどうかを判定
    /// @param  a 値1
    /// @param  b 値2
    /// @param  epsilon 許容誤差
    /// @return ほぼ等しければtrue
    [[nodiscard]] constexpr bool IsNearlyEqual(float a, float b, float epsilon = kEpsilon) noexcept {
        return IsNearlyZero(a - b, epsilon);
    }

    /// @brief  角度を-180°〜180°の範囲に正規化
    /// @param  degrees 角度（度）
    /// @return 正規化された角度
    [[nodiscard]] inline float NormalizeAngleDegrees(float degrees) noexcept {
        // fmod使用のためinline（constexprではない）
        degrees = std::fmod(degrees + 180.0f, 360.0f);
        if (degrees < 0.0f) {
            degrees += 360.0f;
        }
        return degrees - 180.0f;
    }

    /// @brief  角度を-π〜πの範囲に正規化
    /// @param  radians 角度（ラジアン）
    /// @return 正規化された角度
    [[nodiscard]] inline float NormalizeAngleRadians(float radians) noexcept {
        radians = std::fmod(radians + kPi, kTwoPi);
        if (radians < 0.0f) {
            radians += kTwoPi;
        }
        return radians - kPi;
    }

} // namespace Engine::Math
