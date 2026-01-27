/// @file   Light.cpp
/// @brief  光源の基底クラスと派生クラス実装
#include "Light.h"

#include <cmath>
#include <DirectXMath.h>

namespace Engine {

    //============================================================
    // Light (Base class)
    //============================================================

    void Light::SetColor(const Vector3& color) {
        m_color = color;
    }

    Vector3 Light::GetColor() const {
        return m_color;
    }

    void Light::SetIntensity(float intensity) {
        m_intensity = (intensity < 0.0f) ? 0.0f : intensity;
    }

    float Light::GetIntensity() const {
        return m_intensity;
    }

    void Light::Enable() {
        m_isEnabled = true;
    }

    void Light::Disable() {
        m_isEnabled = false;
    }

    bool Light::IsEnabled() const {
        return m_isEnabled;
    }

    void Light::SetCastShadow(bool castShadow) {
        m_castShadow = castShadow;
    }

    bool Light::IsCastShadow() const {
        return m_castShadow;
    }

    void Light::SetAmbient(const Vector3& ambient) {
        m_ambient = ambient;
    }

    Vector3 Light::GetAmbient() const {
        return m_ambient;
    }

    //============================================================
    // DirectionalLightObject
    //============================================================

    void DirectionalLightObject::SetDirection(const Vector3& direction) {
        m_direction = direction.Normalized();
    }

    Vector3 DirectionalLightObject::GetDirection() const {
        return m_direction;
    }

    //============================================================
    // PointLightObject
    //============================================================

    void PointLightObject::SetPosition(const Vector3& position) {
        m_position = position;
    }

    Vector3 PointLightObject::GetPosition() const {
        return m_position;
    }

    void PointLightObject::SetRange(float range) {
        m_range = (range < 0.0f) ? 0.0f : range;
    }

    float PointLightObject::GetRange() const {
        return m_range;
    }

    void PointLightObject::SetAttenuation(float constant, float linear, float quadratic) {
        m_constantAttenuation = constant;
        m_linearAttenuation = linear;
        m_quadraticAttenuation = quadratic;
    }

    void PointLightObject::GetAttenuation(float& outConstant, float& outLinear, float& outQuadratic) const {
        outConstant = m_constantAttenuation;
        outLinear = m_linearAttenuation;
        outQuadratic = m_quadraticAttenuation;
    }

    //============================================================
    // SpotLightObject
    //============================================================

    void SpotLightObject::SetDirection(const Vector3& direction) {
        m_direction = direction.Normalized();
    }

    Vector3 SpotLightObject::GetDirection() const {
        return m_direction;
    }

    void SpotLightObject::SetConeAngles(float innerAngle, float outerAngle) {
        // Ensure inner <= outer
        if (innerAngle > outerAngle) {
            float temp = innerAngle;
            innerAngle = outerAngle;
            outerAngle = temp;
        }

        m_innerAngle = innerAngle;
        m_outerAngle = outerAngle;

        // Convert to radians and compute cosine
        float innerRad = DirectX::XMConvertToRadians(innerAngle);
        float outerRad = DirectX::XMConvertToRadians(outerAngle);

        m_innerCosAngle = std::cos(innerRad);
        m_outerCosAngle = std::cos(outerRad);
    }

    void SpotLightObject::GetConeAngles(float& outInnerAngle, float& outOuterAngle) const {
        outInnerAngle = m_innerAngle;
        outOuterAngle = m_outerAngle;
    }

    float SpotLightObject::GetInnerCosAngle() const {
        return m_innerCosAngle;
    }

    float SpotLightObject::GetOuterCosAngle() const {
        return m_outerCosAngle;
    }

} // namespace Engine
