#pragma once

#include <DirectXMath.h>
#include <cstdint>

namespace Engine {

    /**
     * @brief 4Dベクトル（DirectXMath連携）
     *
     * - 軽量な値型（x,y,z,w public）
     * - XMFLOAT4 / XMVECTOR と相互変換
     * - 主要演算 + Dot/Normalize/Lerp を提供
     */
    struct Vector4 final {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float w = 0.0f;

        //============================================================
        // Constructors
        //============================================================
        constexpr Vector4() noexcept = default;
        constexpr Vector4(float x_, float y_, float z_, float w_) noexcept : x(x_), y(y_), z(z_), w(w_) {}

        explicit Vector4(const DirectX::XMFLOAT4& f4) noexcept : x(f4.x), y(f4.y), z(f4.z), w(f4.w) {}
        explicit Vector4(DirectX::FXMVECTOR v) noexcept;

        //============================================================
        // Conversion
        //============================================================
        DirectX::XMFLOAT4 ToXMFLOAT4() const noexcept { return { x, y, z, w }; }
        DirectX::XMVECTOR ToXMVECTOR() const noexcept { return DirectX::XMVectorSet(x, y, z, w); }

        //============================================================
        // Operators
        //============================================================
        constexpr Vector4 operator+() const noexcept { return *this; }
        constexpr Vector4 operator-() const noexcept { return Vector4(-x, -y, -z, -w); }

        constexpr Vector4 operator+(const Vector4& rhs) const noexcept { return Vector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
        constexpr Vector4 operator-(const Vector4& rhs) const noexcept { return Vector4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
        constexpr Vector4 operator*(float s) const noexcept { return Vector4(x * s, y * s, z * s, w * s); }
        Vector4 operator/(float s) const noexcept;

        Vector4& operator+=(const Vector4& rhs) noexcept { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
        Vector4& operator-=(const Vector4& rhs) noexcept { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
        Vector4& operator*=(float s) noexcept { x *= s; y *= s; z *= s; w *= s; return *this; }
        Vector4& operator/=(float s) noexcept;

        bool operator==(const Vector4& rhs) const noexcept { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
        bool operator!=(const Vector4& rhs) const noexcept { return !(*this == rhs); }

        //============================================================
        // Utility
        //============================================================
        float Length() const noexcept;
        float LengthSquared() const noexcept { return x * x + y * y + z * z + w * w; }

        Vector4 Normalized() const noexcept;
        void NormalizeInPlace() noexcept;

        bool Equals(const Vector4& rhs, float epsilon = kEpsilon) const noexcept;

        //============================================================
        // Static
        //============================================================
        static constexpr Vector4 Zero() noexcept { return Vector4(0, 0, 0, 0); }
        static constexpr Vector4 One() noexcept { return Vector4(1, 1, 1, 1); }

        static float Dot(const Vector4& a, const Vector4& b) noexcept;

        // 色用途でよく使う
        static constexpr Vector4 White() noexcept { return Vector4(1, 1, 1, 1); }
        static constexpr Vector4 Black() noexcept { return Vector4(0, 0, 0, 1); }
        static constexpr Vector4 Red() noexcept { return Vector4(1, 0, 0, 1); }
        static constexpr Vector4 Green() noexcept { return Vector4(0, 1, 0, 1); }
        static constexpr Vector4 Blue() noexcept { return Vector4(0, 0, 1, 1); }

        static Vector4 Lerp(const Vector4& a, const Vector4& b, float t) noexcept;

    public:
        static constexpr float kEpsilon = 1e-6f;
    };

    inline constexpr Vector4 operator*(float s, const Vector4& v) noexcept {
        return v * s;
    }

} // namespace Engine
