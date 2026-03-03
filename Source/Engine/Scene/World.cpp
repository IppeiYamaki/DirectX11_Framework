/// @file   World.cpp
/// @brief  Scene群を統括するWorldクラス実装
#include "World.h"

#include "Engine/Core/Logger.h"
#include "Engine/Physics/PhysicsSystem.h"

namespace Engine {

    World::~World() {
        Finalize();
    }

    //============================================================
    // Lifecycle
    //============================================================

    bool World::Initialize() {
        if (m_isInitialized) return true;

        m_scene = std::make_unique<Scene>();
        if (!m_scene->Initialize()) {
            Logger::Error("World::Initialize failed: Scene initialize failed.");
            Finalize();
            return false;
        }

        // PhysicsSystem の初期化
        m_physicsSystem = std::make_unique<PhysicsSystem>();
        if (!m_physicsSystem->Initialize()) {
            Logger::Error("World::Initialize failed: PhysicsSystem initialize failed.");
            Finalize();
            return false;
        }

        m_isInitialized = true;
        Logger::Info("World initialized.");
        return true;
    }

    void World::Finalize() {
        if (!m_isInitialized) {
            m_physicsSystem.reset();
            m_scene.reset();
            return;
        }

        if (m_physicsSystem) {
            m_physicsSystem->Finalize();
        }
        m_physicsSystem.reset();

        if (m_scene) {
            m_scene->Finalize();
        }
        m_scene.reset();

        m_isInitialized = false;
        Logger::Info("World finalized.");
    }

    void World::Reset() {
        if (!m_isInitialized) return;

        if (m_scene) {
            m_scene->Reset();
        }
    }

    //============================================================
    // Frame
    //============================================================

    void World::Update(float deltaTime) {
        if (!m_isInitialized) return;

        if (m_scene) {
            m_scene->Update(deltaTime);
        }

        // 物理更新（Scene更新後、LateUpdate前に実行）
        if (m_physicsSystem && m_scene) {
            m_physicsSystem->Update(m_scene.get(), deltaTime);
        }
    }

    void World::LateUpdate(float deltaTime) {
        if (!m_isInitialized) return;

        if (m_scene) {
            m_scene->LateUpdate(deltaTime);
        }
    }

    void World::Draw() {
        if (!m_isInitialized) return;

        if (m_scene) {
            m_scene->Draw();
        }
    }

    //============================================================
    // Accessors
    //============================================================

    bool World::IsInitialized() const {
        return m_isInitialized;
    }

    Scene* World::GetScene() {
        return m_scene.get();
    }

    const Scene* World::GetScene() const {
        return m_scene.get();
    }

    PhysicsSystem* World::GetPhysicsSystem() {
        return m_physicsSystem.get();
    }

    const PhysicsSystem* World::GetPhysicsSystem() const {
        return m_physicsSystem.get();
    }

} // namespace Engine
