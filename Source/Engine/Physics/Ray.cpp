/// @file   Ray.cpp
/// @brief  レイ（光線）クラス実装
#include "Ray.h"

namespace Engine {

    Ray::Ray()
        : m_origin(Vector3::Zero())
        , m_direction(Vector3::UnitZ()) {
    }

    Ray::Ray(const Vector3& origin, const Vector3& direction)
        : m_origin(origin)
        , m_direction(direction.Normalized()) {
    }

    //============================================================
    // Origin
    //============================================================

    const Vector3& Ray::GetOrigin() const {
        return m_origin;
    }

    void Ray::SetOrigin(const Vector3& origin) {
        m_origin = origin;
    }

    //============================================================
    // Direction
    //============================================================

    const Vector3& Ray::GetDirection() const {
        return m_direction;
    }

    void Ray::SetDirection(const Vector3& direction) {
        m_direction = direction.Normalized();
    }

    //============================================================
    // Point Calculation
    //============================================================

    Vector3 Ray::GetPoint(float distance) const {
        return m_origin + m_direction * distance;
    }

} // namespace Engine
