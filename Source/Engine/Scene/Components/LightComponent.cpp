/// @file   LightComponent.cpp
/// @brief  ライトをGameObjectにアタッチするためのComponent実装
#include "LightComponent.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/LightSystem.h"

namespace Engine {

    //============================================================
    // LightComponent (Base class)
    //============================================================

    LightComponent::LightComponent(LightSystem* lightSystem)
        : m_lightSystem(lightSystem) {
    }

    LightComponent::~LightComponent() {
        // OnDestroy で登録解除するので、ここでは何もしない
    }

    void LightComponent::OnAwake() {
        RegisterLight();
    }

    void LightComponent::Update([[maybe_unused]] float deltaTime) {
        // Transform位置/方向をライトに同期
        SyncPositionFromTransform();
        SyncDirectionFromTransform();
    }

    void LightComponent::OnDestroy() {
        UnregisterLight();
    }

    //============================================================
    // DirectionalLightComponent
    //============================================================

    DirectionalLightComponent::DirectionalLightComponent(LightSystem* lightSystem, const Vector3& direction)
        : LightComponent(lightSystem)
        , m_initialDirection(direction) {
    }

    void DirectionalLightComponent::RegisterLight() {
        if (!m_lightSystem || m_isRegistered) return;

        m_light = m_lightSystem->AddDirectionalLight(m_initialDirection);
        if (m_light) {
            m_isRegistered = true;
            // Transform方向がある場合は同期、なければ初期方向を使用
            SyncDirectionFromTransform();
            Logger::Info("DirectionalLightComponent: Light registered.");
        }
    }

    void DirectionalLightComponent::UnregisterLight() {
        if (!m_lightSystem || !m_isRegistered || !m_light) return;

        m_lightSystem->RemoveLight(m_light);
        m_light = nullptr;
        m_isRegistered = false;
        Logger::Info("DirectionalLightComponent: Light unregistered.");
    }

    void DirectionalLightComponent::SyncDirectionFromTransform() {
        if (!m_light) return;

        // Transformのforward方向をライトの方向として使用
        auto* owner = GetGameObject();
        if (owner) {
            if (auto* transform = owner->GetComponent<Transform>()) {
                Vector3 forward = transform->GetForward();
                m_light->SetDirection(forward);
            }
        }
    }

    void DirectionalLightComponent::SetDirection(const Vector3& direction) {
        if (m_light) {
            m_light->SetDirection(direction);
        }
    }

    Vector3 DirectionalLightComponent::GetDirection() const {
        return m_light ? m_light->GetDirection() : Vector3(0.0f, -1.0f, 0.0f);
    }

    void DirectionalLightComponent::SetColor(const Vector3& color) {
        if (m_light) m_light->SetColor(color);
    }

    Vector3 DirectionalLightComponent::GetColor() const {
        return m_light ? m_light->GetColor() : Vector3(1.0f, 1.0f, 1.0f);
    }

    void DirectionalLightComponent::SetIntensity(float intensity) {
        if (m_light) m_light->SetIntensity(intensity);
    }

    float DirectionalLightComponent::GetIntensity() const {
        return m_light ? m_light->GetIntensity() : 1.0f;
    }

    void DirectionalLightComponent::SetAmbient(const Vector3& ambient) {
        if (m_light) m_light->SetAmbient(ambient);
    }

    Vector3 DirectionalLightComponent::GetAmbient() const {
        return m_light ? m_light->GetAmbient() : Vector3(0.1f, 0.1f, 0.1f);
    }

    void DirectionalLightComponent::SetCastShadow(bool castShadow) {
        if (m_light) m_light->SetCastShadow(castShadow);
    }

    bool DirectionalLightComponent::IsCastShadow() const {
        return m_light ? m_light->IsCastShadow() : false;
    }

    Light* DirectionalLightComponent::GetLightObject() {
        return m_light;
    }

    const Light* DirectionalLightComponent::GetLightObject() const {
        return m_light;
    }

    //============================================================
    // PointLightComponent
    //============================================================

    PointLightComponent::PointLightComponent(LightSystem* lightSystem)
        : LightComponent(lightSystem) {
    }

    void PointLightComponent::RegisterLight() {
        if (!m_lightSystem || m_isRegistered) return;

        // 初期位置は(0,0,0)、後でTransformから同期
        m_light = m_lightSystem->AddPointLight(Vector3(0.0f, 0.0f, 0.0f));
        if (m_light) {
            m_isRegistered = true;
            SyncPositionFromTransform();
            Logger::Info("PointLightComponent: Light registered.");
        }
    }

    void PointLightComponent::UnregisterLight() {
        if (!m_lightSystem || !m_isRegistered || !m_light) return;

        m_lightSystem->RemoveLight(m_light);
        m_light = nullptr;
        m_isRegistered = false;
        Logger::Info("PointLightComponent: Light unregistered.");
    }

    void PointLightComponent::SyncPositionFromTransform() {
        if (!m_light) return;

        auto* owner = GetGameObject();
        if (owner) {
            if (auto* transform = owner->GetComponent<Transform>()) {
                m_light->SetPosition(transform->GetWorldPosition());
            }
        }
    }

    void PointLightComponent::SetRange(float range) {
        if (m_light) m_light->SetRange(range);
    }

    float PointLightComponent::GetRange() const {
        return m_light ? m_light->GetRange() : 10.0f;
    }

    void PointLightComponent::SetAttenuation(float constant, float linear, float quadratic) {
        if (m_light) m_light->SetAttenuation(constant, linear, quadratic);
    }

    void PointLightComponent::GetAttenuation(float& outConstant, float& outLinear, float& outQuadratic) const {
        if (m_light) {
            m_light->GetAttenuation(outConstant, outLinear, outQuadratic);
        } else {
            outConstant = 1.0f;
            outLinear = 0.09f;
            outQuadratic = 0.032f;
        }
    }

    void PointLightComponent::SetColor(const Vector3& color) {
        if (m_light) m_light->SetColor(color);
    }

    Vector3 PointLightComponent::GetColor() const {
        return m_light ? m_light->GetColor() : Vector3(1.0f, 1.0f, 1.0f);
    }

    void PointLightComponent::SetIntensity(float intensity) {
        if (m_light) m_light->SetIntensity(intensity);
    }

    float PointLightComponent::GetIntensity() const {
        return m_light ? m_light->GetIntensity() : 1.0f;
    }

    void PointLightComponent::SetAmbient(const Vector3& ambient) {
        if (m_light) m_light->SetAmbient(ambient);
    }

    Vector3 PointLightComponent::GetAmbient() const {
        return m_light ? m_light->GetAmbient() : Vector3(0.1f, 0.1f, 0.1f);
    }

    void PointLightComponent::SetCastShadow(bool castShadow) {
        if (m_light) m_light->SetCastShadow(castShadow);
    }

    bool PointLightComponent::IsCastShadow() const {
        return m_light ? m_light->IsCastShadow() : false;
    }

    Light* PointLightComponent::GetLightObject() {
        return m_light;
    }

    const Light* PointLightComponent::GetLightObject() const {
        return m_light;
    }

    //============================================================
    // SpotLightComponent
    //============================================================

    SpotLightComponent::SpotLightComponent(LightSystem* lightSystem,
        const Vector3& direction,
        float innerAngle,
        float outerAngle)
        : LightComponent(lightSystem) {
        // パラメータは RegisterLight 後に設定
        (void)direction;
        (void)innerAngle;
        (void)outerAngle;
    }

    void SpotLightComponent::RegisterLight() {
        if (!m_lightSystem || m_isRegistered) return;

        m_light = m_lightSystem->AddSpotLight(
            Vector3(0.0f, 0.0f, 0.0f),
            Vector3(0.0f, -1.0f, 0.0f),
            30.0f,
            45.0f
        );
        if (m_light) {
            m_isRegistered = true;
            SyncPositionFromTransform();
            SyncDirectionFromTransform();
            Logger::Info("SpotLightComponent: Light registered.");
        }
    }

    void SpotLightComponent::UnregisterLight() {
        if (!m_lightSystem || !m_isRegistered || !m_light) return;

        m_lightSystem->RemoveLight(m_light);
        m_light = nullptr;
        m_isRegistered = false;
        Logger::Info("SpotLightComponent: Light unregistered.");
    }

    void SpotLightComponent::SyncPositionFromTransform() {
        if (!m_light) return;

        auto* owner = GetGameObject();
        if (owner) {
            if (auto* transform = owner->GetComponent<Transform>()) {
                m_light->SetPosition(transform->GetWorldPosition());
            }
        }
    }

    void SpotLightComponent::SyncDirectionFromTransform() {
        if (!m_light) return;

        auto* owner = GetGameObject();
        if (owner) {
            if (auto* transform = owner->GetComponent<Transform>()) {
                Vector3 forward = transform->GetForward();
                m_light->SetDirection(forward);
            }
        }
    }

    void SpotLightComponent::SetDirection(const Vector3& direction) {
        if (m_light) m_light->SetDirection(direction);
    }

    Vector3 SpotLightComponent::GetDirection() const {
        return m_light ? m_light->GetDirection() : Vector3(0.0f, -1.0f, 0.0f);
    }

    void SpotLightComponent::SetRange(float range) {
        if (m_light) m_light->SetRange(range);
    }

    float SpotLightComponent::GetRange() const {
        return m_light ? m_light->GetRange() : 10.0f;
    }

    void SpotLightComponent::SetAttenuation(float constant, float linear, float quadratic) {
        if (m_light) m_light->SetAttenuation(constant, linear, quadratic);
    }

    void SpotLightComponent::GetAttenuation(float& outConstant, float& outLinear, float& outQuadratic) const {
        if (m_light) {
            m_light->GetAttenuation(outConstant, outLinear, outQuadratic);
        } else {
            outConstant = 1.0f;
            outLinear = 0.09f;
            outQuadratic = 0.032f;
        }
    }

    void SpotLightComponent::SetConeAngles(float innerAngle, float outerAngle) {
        if (m_light) m_light->SetConeAngles(innerAngle, outerAngle);
    }

    void SpotLightComponent::GetConeAngles(float& outInnerAngle, float& outOuterAngle) const {
        if (m_light) {
            m_light->GetConeAngles(outInnerAngle, outOuterAngle);
        } else {
            outInnerAngle = 30.0f;
            outOuterAngle = 45.0f;
        }
    }

    void SpotLightComponent::SetColor(const Vector3& color) {
        if (m_light) m_light->SetColor(color);
    }

    Vector3 SpotLightComponent::GetColor() const {
        return m_light ? m_light->GetColor() : Vector3(1.0f, 1.0f, 1.0f);
    }

    void SpotLightComponent::SetIntensity(float intensity) {
        if (m_light) m_light->SetIntensity(intensity);
    }

    float SpotLightComponent::GetIntensity() const {
        return m_light ? m_light->GetIntensity() : 1.0f;
    }

    void SpotLightComponent::SetAmbient(const Vector3& ambient) {
        if (m_light) m_light->SetAmbient(ambient);
    }

    Vector3 SpotLightComponent::GetAmbient() const {
        return m_light ? m_light->GetAmbient() : Vector3(0.1f, 0.1f, 0.1f);
    }

    void SpotLightComponent::SetCastShadow(bool castShadow) {
        if (m_light) m_light->SetCastShadow(castShadow);
    }

    bool SpotLightComponent::IsCastShadow() const {
        return m_light ? m_light->IsCastShadow() : false;
    }

    Light* SpotLightComponent::GetLightObject() {
        return m_light;
    }

    const Light* SpotLightComponent::GetLightObject() const {
        return m_light;
    }

} // namespace Engine
