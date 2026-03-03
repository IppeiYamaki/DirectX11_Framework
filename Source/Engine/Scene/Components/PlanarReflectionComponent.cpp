#include "PlanarReflectionComponent.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Graphics/PlanarReflectionSystem.h"

namespace Engine {

    PlanarReflectionComponent::PlanarReflectionComponent(PlanarReflectionSystem* reflectionSystem)
        : m_reflectionSystem(reflectionSystem) {
    }

    void PlanarReflectionComponent::OnStart() {
        if (!m_reflectionSystem) {
            Logger::Warn("PlanarReflectionComponent::OnStart: reflectionSystem is null.");
            return;
        }

        // 初期反射平面を設定
        Update(0.0f);

        Logger::Info("PlanarReflectionComponent initialized.");
    }

    void PlanarReflectionComponent::Update(float deltaTime) {
        (void)deltaTime;

        if (!m_reflectionSystem) return;

        auto* owner = GetOwner();
        if (!owner) return;

        auto* transform = owner->GetComponent<Transform>();
        if (!transform) return;

        // Transformから反射平面の位置を取得
        Vector3 planePoint = transform->GetPosition() + m_planeOffset;

        // 法線はワールド座標系に変換（Transformの回転を考慮）
        // 簡易実装：ローカル法線をそのまま使用（回転は考慮しない）
        // より正確な実装が必要な場合は、Transformのワールド行列で法線を変換する
        Vector3 planeNormal = m_planeNormal;

        // 反射平面を設定
        ReflectionPlane plane(planePoint, planeNormal);
        m_reflectionSystem->SetReflectionPlane(plane);
    }

} // namespace Engine
