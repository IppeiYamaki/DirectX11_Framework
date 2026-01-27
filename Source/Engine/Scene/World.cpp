#include "World.h"

#include "Engine/Core/Logger.h"
#include "Engine/Core/Assert.h"

#include "Engine/Scene/Entity.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"

#include <algorithm>

namespace Engine {

    World::~World() {
        Finalize();
    }

    //============================================================
    // Lifecycle
    //============================================================

    bool World::Initialize() {
        if (m_isInitialized) return true;

        m_entities.clear();
        m_pendingDestruction.clear();
        m_gameObjects.clear();
        m_pendingGameObjectDestruction.clear();
        m_isInitialized = true;

        Logger::Info("World initialized.");
        return true;
    }

    void World::Finalize() {
        if (!m_isInitialized) {
            m_entities.clear();
            m_pendingDestruction.clear();
            m_gameObjects.clear();
            m_pendingGameObjectDestruction.clear();
            return;
        }

        m_pendingGameObjectDestruction.clear();
        m_gameObjects.clear();
        m_pendingDestruction.clear();
        m_entities.clear();

        m_isInitialized = false;
        Logger::Info("World finalized.");
    }

    void World::Reset() {
        if (!m_isInitialized) return;

        m_pendingGameObjectDestruction.clear();
        m_gameObjects.clear();
        m_pendingDestruction.clear();
        m_entities.clear();
    }

    //============================================================
    // Frame
    //============================================================

    void World::Update(float deltaTime) {
        if (!m_isInitialized) return;

        // Entity更新
        for (auto& e : m_entities) {
            e->UpdateComponents(deltaTime);
        }

        // GameObject更新
        for (auto& obj : m_gameObjects) {
            if (obj->IsActive()) {
                obj->Update(deltaTime);
                obj->UpdateComponents(deltaTime);
            }
        }

        // フレーム終了時に遅延破棄を処理
        ProcessPendingDestructions();
    }

    void World::LateUpdate(float deltaTime) {
        if (!m_isInitialized) return;

        // Entity遅延更新
        for (auto& e : m_entities) {
            e->LateUpdateComponents(deltaTime);
        }

        // GameObject遅延更新
        for (auto& obj : m_gameObjects) {
            if (obj->IsActive()) {
                obj->LateUpdate(deltaTime);
                obj->LateUpdateComponents(deltaTime);
            }
        }
    }

    void World::Draw() {
        if (!m_isInitialized) return;

        // Entity描画
        for (auto& e : m_entities) {
            e->DrawComponents();
        }

        // GameObject描画
        for (auto& obj : m_gameObjects) {
            if (obj->IsActive()) {
                obj->Render();
                obj->DrawComponents();
            }
        }
    }

    bool World::IsInitialized() const {
        return m_isInitialized;
    }

    //============================================================
    // Entity API
    //============================================================

    Entity* World::CreateEntity() {
        if (!m_isInitialized) return nullptr;

        auto entity = std::make_unique<Entity>();
        Entity* raw = entity.get();
        m_entities.emplace_back(std::move(entity));

        raw->AddComponent<Engine::Transform>();

        return raw;
    }

    Entity* World::CreateEntity(const std::string& name) {
        Entity* entity = CreateEntity();
        if (entity) {
            entity->SetName(name);
        }
        return entity;
    }

    void World::DestroyEntity(Entity* entity) {
        if (!m_isInitialized) return;
        if (entity == nullptr) return;

        for (auto it = m_entities.begin(); it != m_entities.end(); ++it) {
            if (it->get() == entity) {
                m_entities.erase(it);
                return;
            }
        }
    }

    void World::DestroyEntityDeferred(Entity* entity) {
        if (!m_isInitialized) return;
        if (entity == nullptr) return;

        // 重複チェック
        auto it = std::find(m_pendingDestruction.begin(), m_pendingDestruction.end(), entity);
        if (it == m_pendingDestruction.end()) {
            m_pendingDestruction.push_back(entity);
        }
    }

    std::uint32_t World::GetEntityCount() const {
        return static_cast<std::uint32_t>(m_entities.size());
    }

    //============================================================
    // GameObject API
    //============================================================

    GameObject* World::AddGameObject(std::unique_ptr<GameObject> object) {
        if (!m_isInitialized) return nullptr;
        if (!object) return nullptr;

        GameObject* raw = object.get();
        raw->SetWorld(this);
        raw->InternalInitialize();
        m_gameObjects.emplace_back(std::move(object));
        return raw;
    }

    void World::DestroyGameObject(GameObject* object) {
        if (!m_isInitialized) return;
        if (object == nullptr) return;

        for (auto it = m_gameObjects.begin(); it != m_gameObjects.end(); ++it) {
            if (it->get() == object) {
                (*it)->OnDestroy();
                (*it)->DestroyComponents();
                m_gameObjects.erase(it);
                return;
            }
        }
    }

    void World::DestroyGameObjectDeferred(GameObject* object) {
        if (!m_isInitialized) return;
        if (object == nullptr) return;

        // 重複チェック
        auto it = std::find(m_pendingGameObjectDestruction.begin(), 
                           m_pendingGameObjectDestruction.end(), object);
        if (it == m_pendingGameObjectDestruction.end()) {
            m_pendingGameObjectDestruction.push_back(object);
        }
    }

    std::uint32_t World::GetGameObjectCount() const {
        return static_cast<std::uint32_t>(m_gameObjects.size());
    }

    //============================================================
    // Entity Query
    //============================================================

    Entity* World::FindEntityByName(const std::string& name) {
        for (auto& e : m_entities) {
            if (e->GetName() == name) {
                return e.get();
            }
        }
        return nullptr;
    }

    Entity* World::FindEntityById(EntityId id) {
        for (auto& e : m_entities) {
            if (e->GetId() == id) {
                return e.get();
            }
        }
        return nullptr;
    }

    std::vector<Entity*> World::FindEntitiesWithTag(const Tag& tag) {
        std::vector<Entity*> result;
        for (auto& e : m_entities) {
            if (e->HasTag(tag)) {
                result.push_back(e.get());
            }
        }
        return result;
    }

    //============================================================
    // GameObject Query
    //============================================================

    GameObject* World::FindGameObjectByName(const std::string& name) {
        for (auto& obj : m_gameObjects) {
            if (obj->GetName() == name) {
                return obj.get();
            }
        }
        return nullptr;
    }

    std::vector<GameObject*> World::FindGameObjectsWithTag(const std::string& tag) {
        std::vector<GameObject*> result;
        for (auto& obj : m_gameObjects) {
            if (obj->CompareTag(tag)) {
                result.push_back(obj.get());
            }
        }
        return result;
    }

    //============================================================
    // Private
    //============================================================

    void World::ProcessPendingDestructions() {
        // Entity遅延破棄
        for (Entity* entity : m_pendingDestruction) {
            DestroyEntity(entity);
        }
        m_pendingDestruction.clear();

        // GameObject遅延破棄
        for (GameObject* object : m_pendingGameObjectDestruction) {
            DestroyGameObject(object);
        }
        m_pendingGameObjectDestruction.clear();
    }

} // namespace Engine
