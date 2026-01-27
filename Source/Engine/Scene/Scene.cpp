#include "Scene.h"

#include "Engine/Core/Logger.h"
#include "Engine/Core/Assert.h"

#include "Engine/Scene/Entity.h"
#include "Engine/Scene/GameObject.h"
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
        m_gameObjects.clear();
        m_pendingObjectDestruction.clear();
        m_isInitialized = true;

        Logger::Info("Scene initialized.");
        return true;
    }

    void Scene::Finalize() {
        if (!m_isInitialized) {
            m_entities.clear();
            m_pendingDestruction.clear();
            m_gameObjects.clear();
            m_pendingObjectDestruction.clear();
            return;
        }

        // GameObjectの破棄処理を呼び出してからクリア
        for (auto& obj : m_gameObjects) {
            obj->OnDestroy();
        }
        m_pendingObjectDestruction.clear();
        m_gameObjects.clear();

        m_pendingDestruction.clear();
        m_entities.clear();

        m_isInitialized = false;
        Logger::Info("Scene finalized.");
    }

    void Scene::Reset() {
        if (!m_isInitialized) return;

        // GameObjectの破棄処理を呼び出してからクリア
        for (auto& obj : m_gameObjects) {
            obj->OnDestroy();
        }
        m_pendingObjectDestruction.clear();
        m_gameObjects.clear();

        m_pendingDestruction.clear();
        m_entities.clear();
    }

    //============================================================
    // Frame
    //============================================================

    void Scene::Update(float deltaTime) {
        if (!m_isInitialized) return;

        // Entity更新
        for (auto& e : m_entities) {
            e->UpdateComponents(deltaTime);
        }

        // GameObject更新
        for (auto& obj : m_gameObjects) {
            obj->Update(deltaTime);
        }

        // フレーム終了時に遅延破棄を処理
        ProcessPendingDestructions();
        ProcessPendingObjectDestructions();
    }

    void Scene::LateUpdate(float deltaTime) {
        if (!m_isInitialized) return;

        // Entity遅延更新
        for (auto& e : m_entities) {
            e->LateUpdateComponents(deltaTime);
        }

        // GameObject遅延更新
        for (auto& obj : m_gameObjects) {
            obj->LateUpdate(deltaTime);
        }
    }

    void Scene::Draw() {
        if (!m_isInitialized) return;

        // Entity描画
        for (auto& e : m_entities) {
            e->DrawComponents();
        }

        // GameObject描画
        for (auto& obj : m_gameObjects) {
            obj->Render();
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
    // GameObject API
    //============================================================

    GameObject* Scene::AddObject(std::unique_ptr<GameObject> object) {
        if (!m_isInitialized) return nullptr;
        if (!object) return nullptr;

        GameObject* raw = object.get();
        raw->SetScene(this);
        raw->Initialize();
        
        m_gameObjects.emplace_back(std::move(object));
        return raw;
    }

    void Scene::DestroyObject(GameObject* object) {
        if (!m_isInitialized) return;
        if (object == nullptr) return;

        for (auto it = m_gameObjects.begin(); it != m_gameObjects.end(); ++it) {
            if (it->get() == object) {
                (*it)->OnDestroy();
                m_gameObjects.erase(it);
                return;
            }
        }
    }

    void Scene::DestroyObjectDeferred(GameObject* object) {
        if (!m_isInitialized) return;
        if (object == nullptr) return;

        // 重複チェック
        auto it = std::find(m_pendingObjectDestruction.begin(), m_pendingObjectDestruction.end(), object);
        if (it == m_pendingObjectDestruction.end()) {
            m_pendingObjectDestruction.push_back(object);
        }
    }

    std::uint32_t Scene::GetObjectCount() const {
        return static_cast<std::uint32_t>(m_gameObjects.size());
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
    // GameObject Query
    //============================================================

    GameObject* Scene::FindObjectByName(const std::string& name) {
        for (auto& obj : m_gameObjects) {
            if (obj->GetName() == name) {
                return obj.get();
            }
        }
        return nullptr;
    }

    GameObject* Scene::FindObjectById(EntityId id) {
        for (auto& obj : m_gameObjects) {
            if (obj->GetId() == id) {
                return obj.get();
            }
        }
        return nullptr;
    }

    std::vector<GameObject*> Scene::FindObjectsWithTag(const std::string& tag) {
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

    void Scene::ProcessPendingDestructions() {
        for (Entity* entity : m_pendingDestruction) {
            DestroyEntity(entity);
        }
        m_pendingDestruction.clear();
    }

    void Scene::ProcessPendingObjectDestructions() {
        for (GameObject* object : m_pendingObjectDestruction) {
            DestroyObject(object);
        }
        m_pendingObjectDestruction.clear();
    }

} // namespace Engine
