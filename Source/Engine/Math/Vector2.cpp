#include "Vector2.h"

#include <cmath> // std::sqrt, std::fabs

namespace Engine {

    Vector2::Vector2(DirectX::FXMVECTOR v) noexcept {
        DirectX::XMFLOAT2 tmp{};
        DirectX::XMStoreFloat2(&tmp, v);
        x = tmp.x;
        y = tmp.y;
    }

    Vector2 Vector2::operator/(float s) const noexcept {
        const float absS = std::fabs(s);
        if (absS <= kEpsilon) return Vector2::Zero();
        const float inv = 1.0f / s;
        return Vector2(x * inv, y * inv);
    }

    Vector2& Vector2::operator/=(float s) noexcept {
        const float absS = std::fabs(s);
        if (absS <= kEpsilon) {
            x = 0.0f; y = 0.0f;
            return *this;
        }
        const float inv = 1.0f / s;
        x *= inv; y *= inv;
        return *this;
    }

    float Vector2::Length() const noexcept {
        return std::sqrt(LengthSquared());
    }

    Vector2 Vector2::Normalized() const noexcept {
        const float lenSq = LengthSquared();
        if (lenSq <= kEpsilon) return Vector2::Zero();
        const float invLen = 1.0f / std::sqrt(lenSq);
        return Vector2(x * invLen, y * invLen);
    }

    void Vector2::NormalizeInPlace() noexcept {
        const float lenSq = LengthSquared();
        if (lenSq <= kEpsilon) {
            x = 0.0f; y = 0.0f;
            return;
        }
        const float invLen = 1.0f / std::sqrt(lenSq);
        x *= invLen; y *= invLen;
    }

    bool Vector2::Equals(const Vector2& rhs, float epsilon) const noexcept {
        return (std::fabs(x - rhs.x) <= epsilon) &&
            (std::fabs(y - rhs.y) <= epsilon);
    }

    float Vector2::Dot(const Vector2& a, const Vector2& b) noexcept {
        return a.x * b.x + a.y * b.y;
    }

    Vector2 Vector2::Lerp(const Vector2& a, const Vector2& b, float t) noexcept {
        return Vector2(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t
        );
    }

} // namespace Engine
