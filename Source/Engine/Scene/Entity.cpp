#include "Entity.h"

#include "Component.h"

namespace Engine {

    Entity::Entity() = default;

    Entity::~Entity() {
        DestroyComponents();
    }

    //============================================================
    // Identification
    //============================================================

    EntityId Entity::GetId() const {
        return m_identity.GetId();
    }

    void Entity::SetName(const std::string& name) {
        m_identity.SetName(name);
    }

    const std::string& Entity::GetName() const {
        return m_identity.GetName();
    }

    void Entity::AddTag(const Tag& tag) {
        m_identity.AddTag(tag);
    }

    void Entity::RemoveTag(const Tag& tag) {
        m_identity.RemoveTag(tag);
    }

    bool Entity::HasTag(const Tag& tag) const {
        return m_identity.HasTag(tag);
    }

    //============================================================
    // Enable
    //============================================================

    void Entity::Enable() {
        m_isEnabled = true;
    }

    void Entity::Disable() {
        m_isEnabled = false;
    }

    bool Entity::IsEnabled() const {
        return m_isEnabled;
    }

    //============================================================
    // Internal (called by World)
    //============================================================

    void Entity::StartIfNeeded() {
        if (m_hasStarted) return;

        for (auto& c : m_components) {
            c->OnStart();
        }
        m_hasStarted = true;
    }

    void Entity::UpdateComponents(float deltaTime) {
        if (!m_isEnabled) return;

        StartIfNeeded();

        for (auto& c : m_components) {
            if (c->IsEnabled()) {
                c->Update(deltaTime);
            }
        }
    }

    void Entity::LateUpdateComponents(float deltaTime) {
        if (!m_isEnabled) return;

        for (auto& c : m_components) {
            if (c->IsEnabled()) {
                c->LateUpdate(deltaTime);
            }
        }
    }

    void Entity::DrawComponents() {
        if (!m_isEnabled) return;

        for (auto& c : m_components) {
            if (c->IsEnabled()) {
                c->Draw();
            }
        }
    }

    void Entity::DestroyComponents() {
        for (auto& c : m_components) {
            c->OnDestroy();
        }
        m_components.clear();
    }

} // namespace Engine
