#include "SampleRotateComponent.h"

#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"

namespace Game {

    SampleRotateComponent::SampleRotateComponent(float degreesPerSecond)
        : m_degreesPerSecond(degreesPerSecond) {
    }

    void SampleRotateComponent::Update(float deltaTime) {
        auto* owner = GetOwner();
        if (!owner) return;

        auto* tr = owner->GetComponent<Engine::Transform>();
        if (!tr) return;

        const float d = m_degreesPerSecond * deltaTime;
        tr->AddRotationEulerDegrees(Engine::Vector3(d, d, d));
    }

} // namespace Game
