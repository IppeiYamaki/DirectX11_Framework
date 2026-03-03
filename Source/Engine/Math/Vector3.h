#pragma once

#include <DirectXMath.h>
#include <cstdint>

namespace Engine {

    /// @brief 3Dベクトル
    struct Vector3 final {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

		//成分アクセス
		float&       X() noexcept       { return x; }
        const float& X() const noexcept { return x; }
		float&       Y() noexcept       { return y; }
		const float& Y() const noexcept { return y; }
		float&       Z() noexcept       { return z; }
		const float& Z() const noexcept { return z; }

        //============================================================
        // Constructors
        //============================================================
        constexpr Vector3() noexcept = default;
        constexpr Vector3(float x_, float y_, float z_) noexcept : x(x_), y(y_), z(z_) {}

        explicit Vector3(const DirectX::XMFLOAT3& f3) noexcept : x(f3.x), y(f3.y), z(f3.z) {}

        // XMVECTOR -> Vector3（安全に一旦XMFLOAT3へ）
        explicit Vector3(DirectX::FXMVECTOR v) noexcept;

        //============================================================
        // Conversion
        //============================================================
        DirectX::XMFLOAT3 ToXMFLOAT3() const noexcept { return { x, y, z }; }
        DirectX::XMVECTOR ToXMVECTOR(float w = 0.0f) const noexcept { return DirectX::XMVectorSet(x, y, z, w); }

        //============================================================
        // Operators
        //============================================================
        constexpr Vector3 operator+() const noexcept { return *this; }
        constexpr Vector3 operator-() const noexcept { return Vector3(-x, -y, -z); }

        constexpr Vector3 operator+(const Vector3& rhs) const noexcept { return Vector3(x + rhs.x, y + rhs.y, z + rhs.z); }
        constexpr Vector3 operator-(const Vector3& rhs) const noexcept { return Vector3(x - rhs.x, y - rhs.y, z - rhs.z); }
        constexpr Vector3 operator*(float s) const noexcept { return Vector3(x * s, y * s, z * s); }
        Vector3 operator/(float s) const noexcept;

        Vector3& operator+=(const Vector3& rhs) noexcept { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
        Vector3& operator-=(const Vector3& rhs) noexcept { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
        Vector3& operator*=(float s) noexcept { x *= s; y *= s; z *= s; return *this; }
        Vector3& operator/=(float s) noexcept;

        // 厳密一致（必要なら使用）。誤差込み比較は Equals() を使う。
        bool operator==(const Vector3& rhs) const noexcept { return x == rhs.x && y == rhs.y && z == rhs.z; }
        bool operator!=(const Vector3& rhs) const noexcept { return !(*this == rhs); }

        //============================================================
        // Utility
        //============================================================
        float Length() const noexcept;
        float LengthSquared() const noexcept { return x * x + y * y + z * z; }

        Vector3 Normalized() const noexcept;
        void NormalizeInPlace() noexcept;

        bool Equals(const Vector3& rhs, float epsilon = kEpsilon) const noexcept;

        //============================================================
        // Static
        //============================================================
        static constexpr Vector3 Zero() noexcept { return Vector3(0.0f, 0.0f, 0.0f); }
        static constexpr Vector3 One()  noexcept { return Vector3(1.0f, 1.0f, 1.0f); }

        static constexpr Vector3 UnitX() noexcept { return Vector3(1.0f, 0.0f, 0.0f); }
        static constexpr Vector3 UnitY() noexcept { return Vector3(0.0f, 1.0f, 0.0f); }
        static constexpr Vector3 UnitZ() noexcept { return Vector3(0.0f, 0.0f, 1.0f); }

        static float Dot(const Vector3& a, const Vector3& b) noexcept;
        static Vector3 Cross(const Vector3& a, const Vector3& b) noexcept;

        static Vector3 Lerp(const Vector3& a, const Vector3& b, float t) noexcept;

    public:
        static constexpr float kEpsilon = 1e-6f;
    };

    // scalar * Vector3
    inline constexpr Vector3 operator*(float s, const Vector3& v) noexcept {
        return v * s;
    }

} // namespace Engine
