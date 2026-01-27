#include "Component.h"

namespace Engine {

    GameObject* Component::GetOwner() {
        return m_owner;
    }

    const GameObject* Component::GetOwner() const {
        return m_owner;
    }

    GameObject* Component::GetGameObject() {
        return m_owner;
    }

    const GameObject* Component::GetGameObject() const {
        return m_owner;
    }

    void Component::Enable() {
        m_isEnabled = true;
    }

    void Component::Disable() {
        m_isEnabled = false;
    }

    bool Component::IsEnabled() const {
        return m_isEnabled;
    }

    void Component::OnAwake() {}
    void Component::OnStart() {}
    void Component::Update(float deltaTime) { (void)deltaTime; }
    void Component::LateUpdate(float deltaTime) { (void)deltaTime; }
    void Component::Draw() {}
    void Component::OnDestroy() {}

    void Component::SetOwner(GameObject* owner) {
        m_owner = owner;
    }

} // namespace Engine
