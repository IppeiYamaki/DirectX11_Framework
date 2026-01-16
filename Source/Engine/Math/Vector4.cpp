#include "Vector4.h"

#include <cmath> // std::sqrt, std::fabs

namespace Engine {

    Vector4::Vector4(DirectX::FXMVECTOR v) noexcept {
        DirectX::XMFLOAT4 tmp{};
        DirectX::XMStoreFloat4(&tmp, v);
        x = tmp.x;
        y = tmp.y;
        z = tmp.z;
        w = tmp.w;
    }

    Vector4 Vector4::operator/(float s) const noexcept {
        const float absS = std::fabs(s);
        if (absS <= kEpsilon) {
            return Vector4::Zero();
        }
        const float inv = 1.0f / s;
        return Vector4(x * inv, y * inv, z * inv, w * inv);
    }

    Vector4& Vector4::operator/=(float s) noexcept {
        const float absS = std::fabs(s);
        if (absS <= kEpsilon) {
            x = 0.0f; y = 0.0f; z = 0.0f; w = 0.0f;
            return *this;
        }
        const float inv = 1.0f / s;
        x *= inv; y *= inv; z *= inv; w *= inv;
        return *this;
    }

    float Vector4::Length() const noexcept {
        return std::sqrt(LengthSquared());
    }

    Vector4 Vector4::Normalized() const noexcept {
        const float lenSq = LengthSquared();
        if (lenSq <= kEpsilon) return Vector4::Zero();

        const float invLen = 1.0f / std::sqrt(lenSq);
        return Vector4(x * invLen, y * invLen, z * invLen, w * invLen);
    }

    void Vector4::NormalizeInPlace() noexcept {
        const float lenSq = LengthSquared();
        if (lenSq <= kEpsilon) {
            x = 0.0f; y = 0.0f; z = 0.0f; w = 0.0f;
            return;
        }
        const float invLen = 1.0f / std::sqrt(lenSq);
        x *= invLen; y *= invLen; z *= invLen; w *= invLen;
    }

    bool Vector4::Equals(const Vector4& rhs, float epsilon) const noexcept {
        return (std::fabs(x - rhs.x) <= epsilon) &&
            (std::fabs(y - rhs.y) <= epsilon) &&
            (std::fabs(z - rhs.z) <= epsilon) &&
            (std::fabs(w - rhs.w) <= epsilon);
    }

    float Vector4::Dot(const Vector4& a, const Vector4& b) noexcept {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }

    Vector4 Vector4::Lerp(const Vector4& a, const Vector4& b, float t) noexcept {
        return Vector4(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t,
            a.w + (b.w - a.w) * t
        );
    }

} // namespace Engine
