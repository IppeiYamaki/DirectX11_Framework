#include "Vector3.h"

#include <cmath> // std::sqrt, std::fabs

namespace Engine {

    Vector3::Vector3(DirectX::FXMVECTOR v) noexcept {
        DirectX::XMFLOAT3 tmp{};
        DirectX::XMStoreFloat3(&tmp, v);
        x = tmp.x;
        y = tmp.y;
        z = tmp.z;
    }

    Vector3 Vector3::operator/(float s) const noexcept {
        const float absS = std::fabs(s);
        if (absS <= kEpsilon) {
            // 0œŽZ‚ð”ð‚¯‚éF‚Æ‚è‚ ‚¦‚¸ƒ[ƒ‚ð•Ô‚·i•K—v‚È‚ç•ûj‚ð•Ï‚¦‚ÄOKj
            return Vector3::Zero();
        }
        const float inv = 1.0f / s;
        return Vector3(x * inv, y * inv, z * inv);
    }

    Vector3& Vector3::operator/=(float s) noexcept {
        const float absS = std::fabs(s);
        if (absS <= kEpsilon) {
            x = 0.0f; y = 0.0f; z = 0.0f;
            return *this;
        }
        const float inv = 1.0f / s;
        x *= inv; y *= inv; z *= inv;
        return *this;
    }

    float Vector3::Length() const noexcept {
        return std::sqrt(LengthSquared());
    }

    Vector3 Vector3::Normalized() const noexcept {
        const float lenSq = LengthSquared();
        if (lenSq <= kEpsilon) return Vector3::Zero();

        const float invLen = 1.0f / std::sqrt(lenSq);
        return Vector3(x * invLen, y * invLen, z * invLen);
    }

    void Vector3::NormalizeInPlace() noexcept {
        const float lenSq = LengthSquared();
        if (lenSq <= kEpsilon) {
            x = 0.0f; y = 0.0f; z = 0.0f;
            return;
        }
        const float invLen = 1.0f / std::sqrt(lenSq);
        x *= invLen; y *= invLen; z *= invLen;
    }

    bool Vector3::Equals(const Vector3& rhs, float epsilon) const noexcept {
        return (std::fabs(x - rhs.x) <= epsilon) &&
            (std::fabs(y - rhs.y) <= epsilon) &&
            (std::fabs(z - rhs.z) <= epsilon);
    }

    float Vector3::Dot(const Vector3& a, const Vector3& b) noexcept {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    Vector3 Vector3::Cross(const Vector3& a, const Vector3& b) noexcept {
        return Vector3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }

    Vector3 Vector3::Lerp(const Vector3& a, const Vector3& b, float t) noexcept {
        return Vector3(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t
        );
    }

} // namespace Engine
