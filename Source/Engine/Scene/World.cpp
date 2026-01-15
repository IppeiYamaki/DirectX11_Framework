#include "World.h"

#include "Engine/Core/Logger.h"
#include "Engine/Core/Assert.h"

#include "Engine/Scene/Entity.h"
#include "Engine/Scene/Components/Transform.h"


namespace Engine {

    World::~World() {
        Finalize();
    }

    bool World::Initialize() {
        if (m_isInitialized) return true;

        m_entities.clear();
        m_isInitialized = true;

        Logger::Info("World initialized.");
        return true;
    }

    void World::Finalize() {
        if (!m_isInitialized) {
            m_entities.clear();
            return;
        }

        // Entity側のデストラクタで Component::OnDestroy → 破棄まで行われる
        m_entities.clear();

        m_isInitialized = false;
        Logger::Info("World finalized.");
    }

    void World::Reset() {
        if (!m_isInitialized) return;

        // リトライ想定：Worldを空に戻す
        m_entities.clear();
    }

    void World::Update(float deltaTime) {
        if (!m_isInitialized) return;

        // Update中に DestroyEntity を即時で呼ぶとイテレータ破壊の危険があるので、
        // 将来的には「破棄予約→ループ後に削除」が推奨。
        // 今は学習段階として、Update中に DestroyEntity を呼ばない運用にしておく。
        for (auto& e : m_entities) {
            e->UpdateComponents(deltaTime);
        }
    }

    void World::LateUpdate(float deltaTime) {
        if (!m_isInitialized) return;

        for (auto& e : m_entities) {
            e->LateUpdateComponents(deltaTime);
        }
    }

    void World::Draw() {
        if (!m_isInitialized) return;

        for (auto& e : m_entities) {
            e->DrawComponents();
        }
    }

    bool World::IsInitialized() const {
        return m_isInitialized;
    }

    Entity* World::CreateEntity() {
        if (!m_isInitialized) return nullptr;

        auto entity = std::make_unique<Entity>();
        Entity* raw = entity.get();
        m_entities.emplace_back(std::move(entity));

        raw->AddComponent<Engine::Transform>(); // Transform必須化

        return raw;
    }


    void World::DestroyEntity(Entity* entity) {
        if (!m_isInitialized) return;
        if (entity == nullptr) return;

        // 今は即時削除（Update中に呼ぶと危険なので注意）
        for (auto it = m_entities.begin(); it != m_entities.end(); ++it) {
            if (it->get() == entity) {
                m_entities.erase(it);
                return;
            }
        }
    }

    std::uint32_t World::GetEntityCount() const {
        return static_cast<std::uint32_t>(m_entities.size());
    }

} // namespace Engine
