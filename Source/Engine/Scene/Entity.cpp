#include "Entity.h"

#include "Component.h"

namespace Engine {

    Entity::Entity() = default;

    Entity::~Entity() {
        DestroyComponents();
    }

    void Entity::Enable() {
        m_isEnabled = true;
    }

    void Entity::Disable() {
        m_isEnabled = false;
    }

    bool Entity::IsEnabled() const {
        return m_isEnabled;
    }

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
        // OnDestroy ‚ðŒÄ‚ñ‚Å‚©‚ç”jŠü
        for (auto& c : m_components) {
            c->OnDestroy();
        }
        m_components.clear();
    }

} // namespace Engine
