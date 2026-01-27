#include "Scene.h"

#include "Engine/Core/Logger.h"
#include "Engine/Core/Assert.h"

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

        m_gameObjects.clear();
        m_pendingDestruction.clear();
        m_isInitialized = true;

        Logger::Info("Scene initialized.");
        return true;
    }

    void Scene::Finalize() {
        if (!m_isInitialized) {
            m_gameObjects.clear();
            m_pendingDestruction.clear();
            return;
        }

        // GameObjectの破棄処理を呼び出してからクリア
        for (auto& obj : m_gameObjects) {
            obj->OnDestroy();
        }
        m_pendingDestruction.clear();
        m_gameObjects.clear();

        m_isInitialized = false;
        Logger::Info("Scene finalized.");
    }

    void Scene::Reset() {
        if (!m_isInitialized) return;

        // GameObjectの破棄処理を呼び出してからクリア
        for (auto& obj : m_gameObjects) {
            obj->OnDestroy();
        }
        m_pendingDestruction.clear();
        m_gameObjects.clear();
    }

    //============================================================
    // Frame
    //============================================================

    void Scene::Update(float deltaTime) {
        if (!m_isInitialized) return;

        // GameObject更新
        for (auto& obj : m_gameObjects) {
            obj->Update(deltaTime);
        }

        // フレーム終了時に遅延破棄を処理
        ProcessPendingDestructions();
    }

    void Scene::LateUpdate(float deltaTime) {
        if (!m_isInitialized) return;

        // GameObject遅延更新
        for (auto& obj : m_gameObjects) {
            obj->LateUpdate(deltaTime);
        }
    }

    void Scene::Draw() {
        if (!m_isInitialized) return;

        // GameObject描画
        for (auto& obj : m_gameObjects) {
            obj->Render();
        }
    }

    bool Scene::IsInitialized() const {
        return m_isInitialized;
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

    GameObject* Scene::CreateEntity() {
        return CreateObject<GameObject>();
    }

    GameObject* Scene::CreateEntity(const std::string& name) {
        return CreateObject<GameObject>(name);
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

    void Scene::DestroyEntity(GameObject* object) {
        DestroyObject(object);
    }

    void Scene::DestroyObjectDeferred(GameObject* object) {
        if (!m_isInitialized) return;
        if (object == nullptr) return;

        // 重複チェック
        auto it = std::find(m_pendingDestruction.begin(), m_pendingDestruction.end(), object);
        if (it == m_pendingDestruction.end()) {
            m_pendingDestruction.push_back(object);
        }
    }

    void Scene::DestroyEntityDeferred(GameObject* object) {
        DestroyObjectDeferred(object);
    }

    std::uint32_t Scene::GetObjectCount() const {
        return static_cast<std::uint32_t>(m_gameObjects.size());
    }

    std::uint32_t Scene::GetEntityCount() const {
        return GetObjectCount();
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

    GameObject* Scene::FindEntityByName(const std::string& name) {
        return FindObjectByName(name);
    }

    GameObject* Scene::FindObjectById(EntityId id) {
        for (auto& obj : m_gameObjects) {
            if (obj->GetId() == id) {
                return obj.get();
            }
        }
        return nullptr;
    }

    GameObject* Scene::FindEntityById(EntityId id) {
        return FindObjectById(id);
    }

    std::vector<GameObject*> Scene::FindObjectsWithTag(const std::string& tag) {
        std::vector<GameObject*> result;
        for (auto& obj : m_gameObjects) {
            if (obj->HasTag(tag)) {
                result.push_back(obj.get());
            }
        }
        return result;
    }

    std::vector<GameObject*> Scene::FindEntitiesWithTag(const Tag& tag) {
        return FindObjectsWithTag(tag);
    }

    //============================================================
    // Private
    //============================================================

    void Scene::ProcessPendingDestructions() {
        for (GameObject* object : m_pendingDestruction) {
            DestroyObject(object);
        }
        m_pendingDestruction.clear();
    }

} // namespace Engine
