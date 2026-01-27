#include "Scene.h"

#include "Engine/Core/Logger.h"
#include "Engine/Core/Assert.h"

#include "Engine/Scene/Entity.h"
#include "Engine/Scene/Components/Transform.h"

#include <algorithm>

namespace Engine {

    Scene::~Scene() {
        Finalize();
    }

    //============================================================
    // Lifecycle
    //============================================================

    bool Scene::Initialize() {
        if (m_isInitialized) return true;

        m_entities.clear();
        m_pendingDestruction.clear();
        m_isInitialized = true;

        Logger::Info("Scene initialized.");
        return true;
    }

    void Scene::Finalize() {
        if (!m_isInitialized) {
            m_entities.clear();
            m_pendingDestruction.clear();
            return;
        }

        m_pendingDestruction.clear();
        m_entities.clear();

        m_isInitialized = false;
        Logger::Info("Scene finalized.");
    }

    void Scene::Reset() {
        if (!m_isInitialized) return;

        m_pendingDestruction.clear();
        m_entities.clear();
    }

    //============================================================
    // Frame
    //============================================================

    void Scene::Update(float deltaTime) {
        if (!m_isInitialized) return;

        for (auto& e : m_entities) {
            e->UpdateComponents(deltaTime);
        }

        // フレーム終了時に遅延破棄を処理
        ProcessPendingDestructions();
    }

    void Scene::LateUpdate(float deltaTime) {
        if (!m_isInitialized) return;

        for (auto& e : m_entities) {
            e->LateUpdateComponents(deltaTime);
        }
    }

    void Scene::Draw() {
        if (!m_isInitialized) return;

        for (auto& e : m_entities) {
            e->DrawComponents();
        }
    }

    bool Scene::IsInitialized() const {
        return m_isInitialized;
    }

    //============================================================
    // Entity API
    //============================================================

    Entity* Scene::CreateEntity() {
        if (!m_isInitialized) return nullptr;

        auto entity = std::make_unique<Entity>();
        Entity* raw = entity.get();
        m_entities.emplace_back(std::move(entity));

        raw->AddComponent<Engine::Transform>();

        return raw;
    }

    Entity* Scene::CreateEntity(const std::string& name) {
        Entity* entity = CreateEntity();
        if (entity) {
            entity->SetName(name);
        }
        return entity;
    }

    void Scene::DestroyEntity(Entity* entity) {
        if (!m_isInitialized) return;
        if (entity == nullptr) return;

        for (auto it = m_entities.begin(); it != m_entities.end(); ++it) {
            if (it->get() == entity) {
                m_entities.erase(it);
                return;
            }
        }
    }

    void Scene::DestroyEntityDeferred(Entity* entity) {
        if (!m_isInitialized) return;
        if (entity == nullptr) return;

        // 重複チェック
        auto it = std::find(m_pendingDestruction.begin(), m_pendingDestruction.end(), entity);
        if (it == m_pendingDestruction.end()) {
            m_pendingDestruction.push_back(entity);
        }
    }

    std::uint32_t Scene::GetEntityCount() const {
        return static_cast<std::uint32_t>(m_entities.size());
    }

    //============================================================
    // Entity Query
    //============================================================

    Entity* Scene::FindEntityByName(const std::string& name) {
        for (auto& e : m_entities) {
            if (e->GetName() == name) {
                return e.get();
            }
        }
        return nullptr;
    }

    Entity* Scene::FindEntityById(EntityId id) {
        for (auto& e : m_entities) {
            if (e->GetId() == id) {
                return e.get();
            }
        }
        return nullptr;
    }

    std::vector<Entity*> Scene::FindEntitiesWithTag(const Tag& tag) {
        std::vector<Entity*> result;
        for (auto& e : m_entities) {
            if (e->HasTag(tag)) {
                result.push_back(e.get());
            }
        }
        return result;
    }

    //============================================================
    // Private
    //============================================================

    void Scene::ProcessPendingDestructions() {
        for (Entity* entity : m_pendingDestruction) {
            DestroyEntity(entity);
        }
        m_pendingDestruction.clear();
    }

} // namespace Engine
