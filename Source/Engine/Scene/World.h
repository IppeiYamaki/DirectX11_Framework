#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "Engine/Scene/Entity.h"

namespace Engine {

    /**
     * @brief EntityŒQ‚ğŠÇ—‚µAXV/•`‰æ‚ğ“Š‡‚·‚é
     *
     * - Update/LateUpdate/Draw ‚ğ Entity¨Component ‚Ö“`”d
     * - ”jŠü‚ÌˆÀ‘SŠÇ—i’x‰„”jŠüj‚Í«—ˆŠg’£i¡‚Í‘¦íœj
     */
    class World final {
    public:
        World() = default;
        ~World();

        World(const World&) = delete;
        World& operator=(const World&) = delete;

        //============================================================
        // Lifecycle
        //============================================================
        bool Initialize();
        void Finalize();
        void Reset();

        //============================================================
        // Frame
        //============================================================
        void Update(float deltaTime);
        void LateUpdate(float deltaTime);
        void Draw();

        bool IsInitialized() const;

        //============================================================
        // Entity API
        //============================================================
        Entity* CreateEntity();
        void DestroyEntity(Entity* entity); // ¡‚Í‘¦íœi«—ˆF’x‰„”jŠüj

        std::uint32_t GetEntityCount() const;

    private:
		bool                                    m_isInitialized = false;    // ‰Šú‰»Ï‚İƒtƒ‰ƒO

		std::vector<std::unique_ptr<Entity>>    m_entities;                 // Š—LEntityŒQ
    };

} // namespace Engine
