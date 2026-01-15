#pragma once

#include <DirectXMath.h>

namespace Engine {

    struct Vector2 final {
        float x = 0.0f;
        float y = 0.0f;

        constexpr Vector2() noexcept = default;
        constexpr Vector2(float x_, float y_) noexcept : x(x_), y(y_) {}

        explicit Vector2(const DirectX::XMFLOAT2& f2) noexcept : x(f2.x), y(f2.y) {}
        explicit Vector2(DirectX::FXMVECTOR v) noexcept;

        DirectX::XMFLOAT2 ToXMFLOAT2() const noexcept { return { x, y }; }
        DirectX::XMVECTOR ToXMVECTOR(float z = 0.0f, float w = 0.0f) const noexcept { return DirectX::XMVectorSet(x, y, z, w); }

        constexpr Vector2 operator-() const noexcept { return Vector2(-x, -y); }
        constexpr Vector2 operator+(const Vector2& rhs) const noexcept { return Vector2(x + rhs.x, y + rhs.y); }
        constexpr Vector2 operator-(const Vector2& rhs) const noexcept { return Vector2(x - rhs.x, y - rhs.y); }
        constexpr Vector2 operator*(float s) const noexcept { return Vector2(x * s, y * s); }
        Vector2 operator/(float s) const noexcept;

        Vector2& operator+=(const Vector2& rhs) noexcept { x += rhs.x; y += rhs.y; return *this; }
        Vector2& operator-=(const Vector2& rhs) noexcept { x -= rhs.x; y -= rhs.y; return *this; }
        Vector2& operator*=(float s) noexcept { x *= s; y *= s; return *this; }
        Vector2& operator/=(float s) noexcept;

        float Length() const noexcept;
        float LengthSquared() const noexcept { return x * x + y * y; }

        Vector2 Normalized() const noexcept;
        void NormalizeInPlace() noexcept;

        bool Equals(const Vector2& rhs, float epsilon = kEpsilon) const noexcept;

        static constexpr Vector2 Zero() noexcept { return Vector2(0, 0); }
        static constexpr Vector2 One() noexcept { return Vector2(1, 1); }

        static float Dot(const Vector2& a, const Vector2& b) noexcept;
        static Vector2 Lerp(const Vector2& a, const Vector2& b, float t) noexcept;

        static constexpr float kEpsilon = 1e-6f;
    };

    inline constexpr Vector2 operator*(float s, const Vector2& v) noexcept {
        return v * s;
    }

} // namespace Engine
