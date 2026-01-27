/// @file   MathConstants.h
/// @brief  数学定数とコンパイル時計算関数
#pragma once

#include <numbers>
#include <cmath>

namespace Engine::Math {

    //============================================================
    // 数学定数 (C++20 std::numbers)
    //============================================================

    /// @brief 円周率 (π)
    inline constexpr float kPi = std::numbers::pi_v<float>;

    /// @brief 2π
    inline constexpr float kTwoPi = 2.0f * kPi;

    /// @brief π/2
    inline constexpr float kHalfPi = kPi / 2.0f;

    /// @brief π/4
    inline constexpr float kQuarterPi = kPi / 4.0f;

    /// @brief 自然対数の底 (e)
    inline constexpr float kE = std::numbers::e_v<float>;

    /// @brief √2
    inline constexpr float kSqrt2 = std::numbers::sqrt2_v<float>;

    /// @brief √3
    inline constexpr float kSqrt3 = std::numbers::sqrt3_v<float>;

    /// @brief 黄金比 (φ)
    inline constexpr float kPhi = std::numbers::phi_v<float>;

    //============================================================
    // 変換定数
    //============================================================

    /// @brief 度からラジアンへの変換係数
    inline constexpr float kDegToRad = kPi / 180.0f;

    /// @brief ラジアンから度への変換係数
    inline constexpr float kRadToDeg = 180.0f / kPi;

    //============================================================
    // 許容誤差
    //============================================================

    /// @brief float比較用の許容誤差
    inline constexpr float kEpsilon = 1e-6f;

    /// @brief ゼロ判定用の閾値
    inline constexpr float kNearZero = 1e-8f;

    //============================================================
    // コンパイル時計算関数
    //============================================================

    /// @brief 度をラジアンに変換
    /// @param degrees 角度（度）
    /// @return 角度（ラジアン）
    [[nodiscard]] constexpr float DegreesToRadians(float degrees) noexcept {
        return degrees * kDegToRad;
    }

    /// @brief ラジアンを度に変換
    /// @param radians 角度（ラジアン）
    /// @return 角度（度）
    [[nodiscard]] constexpr float RadiansToDegrees(float radians) noexcept {
        return radians * kRadToDeg;
    }

    /// @brief 値を範囲内にクランプ
    /// @param value クランプする値
    /// @param minVal 最小値
    /// @param maxVal 最大値
    /// @return クランプされた値
    [[nodiscard]] constexpr float Clamp(float value, float minVal, float maxVal) noexcept {
        if (value < minVal) return minVal;
        if (value > maxVal) return maxVal;
        return value;
    }

    /// @brief 0から1の範囲にクランプ
    /// @param value クランプする値
    /// @return クランプされた値
    [[nodiscard]] constexpr float Saturate(float value) noexcept {
        return Clamp(value, 0.0f, 1.0f);
    }

    /// @brief 線形補間
    /// @param a 開始値
    /// @param b 終了値
    /// @param t 補間係数 (0.0 - 1.0)
    /// @return 補間された値
    [[nodiscard]] constexpr float Lerp(float a, float b, float t) noexcept {
        return a + (b - a) * t;
    }

    /// @brief 逆線形補間（aとbの間でvalueがどの位置にあるか）
    /// @param a     開始値
    /// @param b     終了値
    /// @param value 評価する値
    /// @return 補間係数
    [[nodiscard]] constexpr float InverseLerp(float a, float b, float value) noexcept {
        if (a == b) return 0.0f;
        return (value - a) / (b - a);
    }

    /// @brief 値のリマップ
    /// @param value      リマップする値
    /// @param inMin      入力範囲の最小値
    /// @param inMax      入力範囲の最大値
    /// @param outMin     出力範囲の最小値
    /// @param outMax     出力範囲の最大値
    /// @return リマップされた値
    [[nodiscard]] constexpr float Remap(float value, float inMin, float inMax,
                                         float outMin, float outMax) noexcept {
        float t = InverseLerp(inMin, inMax, value);
        return Lerp(outMin, outMax, t);
    }

    /// @brief 角度を0〜360度の範囲に正規化
    /// @param degrees 角度（度）
    /// @return 正規化された角度
    [[nodiscard]] constexpr float NormalizeAngle360(float degrees) noexcept {
        degrees = std::fmod(degrees, 360.0f);
        if (degrees < 0.0f) {
            degrees += 360.0f;
        }
        return degrees;
    }

    /// @brief 角度を-180〜180度の範囲に正規化
    /// @param degrees 角度（度）
    /// @return 正規化された角度
    [[nodiscard]] constexpr float NormalizeAngle180(float degrees) noexcept {
        degrees = NormalizeAngle360(degrees);
        if (degrees > 180.0f) {
            degrees -= 360.0f;
        }
        return degrees;
    }

    /// @brief 2つのfloatがほぼ等しいかチェック
    /// @param a       比較する値1
    /// @param b       比較する値2
    /// @param epsilon 許容誤差
    /// @return ほぼ等しければtrue
    [[nodiscard]] constexpr bool IsNearlyEqual(float a, float b,
                                                float epsilon = kEpsilon) noexcept {
        return std::abs(a - b) <= epsilon;
    }

    /// @brief 値がほぼゼロかチェック
    /// @param value     チェックする値
    /// @param threshold 閾値
    /// @return ほぼゼロならtrue
    [[nodiscard]] constexpr bool IsNearlyZero(float value,
                                               float threshold = kNearZero) noexcept {
        return std::abs(value) <= threshold;
    }

    /// @brief 符号を返す
    /// @param value 評価する値
    /// @return -1, 0, or 1
    [[nodiscard]] constexpr float Sign(float value) noexcept {
        if (value > 0.0f) return 1.0f;
        if (value < 0.0f) return -1.0f;
        return 0.0f;
    }

    /// @brief 2つの値の最小値
    /// @param a 値1
    /// @param b 値2
    /// @return 小さい方の値
    [[nodiscard]] constexpr float Min(float a, float b) noexcept {
        return (a < b) ? a : b;
    }

    /// @brief 2つの値の最大値
    /// @param a 値1
    /// @param b 値2
    /// @return 大きい方の値
    [[nodiscard]] constexpr float Max(float a, float b) noexcept {
        return (a > b) ? a : b;
    }

    /// @brief 絶対値
    /// @param value 値
    /// @return 絶対値
    [[nodiscard]] constexpr float Abs(float value) noexcept {
        return (value < 0.0f) ? -value : value;
    }

    //============================================================
    // コンパイル時のみ評価される関数 (consteval)
    //============================================================

    /// @brief コンパイル時に角度をラジアンに変換
    /// @param degrees 角度（度）
    /// @return 角度（ラジアン）
    /// @note この関数はコンパイル時にのみ評価可能
    [[nodiscard]] consteval float CompileTimeRadians(float degrees) noexcept {
        return degrees * kDegToRad;
    }

    /// @brief コンパイル時に度に変換
    /// @param radians 角度（ラジアン）
    /// @return 角度（度）
    /// @note この関数はコンパイル時にのみ評価可能
    [[nodiscard]] consteval float CompileTimeDegrees(float radians) noexcept {
        return radians * kRadToDeg;
    }

} // namespace Engine::Math

// 使用例:
// constexpr float angle90 = Engine::Math::DegreesToRadians(90.0f);
// constexpr float halfwayValue = Engine::Math::Lerp(0.0f, 100.0f, 0.5f); // 50.0f
// constexpr float clamped = Engine::Math::Clamp(150.0f, 0.0f, 100.0f);   // 100.0f
