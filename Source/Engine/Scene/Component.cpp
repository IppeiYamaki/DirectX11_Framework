#include "Component.h"

namespace Engine {

    Entity* Component::GetOwner() {
        return m_owner;
    }

    const Entity* Component::GetOwner() const {
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

    void Component::SetOwner(Entity* owner) {
        m_owner = owner;
    }

} // namespace Engine
