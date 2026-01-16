#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include "Materials/MaterialBuildContext.h"

namespace Engine { class Material; }

namespace Game {

    /**
     * @brief gMaterial‘Yh ‚ğ¶¬‚µ‚ÄƒLƒƒƒbƒVƒ…‚·‚é
     * - SampleCubeMaterial ‚È‚Ç‚Ì gMaterialƒNƒ‰ƒX(h/cpp)h ‚ğ‚±‚±‚Åg‚¢‰ñ‚·
     */
    class MaterialLibrary final {
    public:
        MaterialLibrary() = default;
        ~MaterialLibrary() = default;

        void Initialize(const MaterialBuildContext& ctx);
        void Finalize();

        template<class TMaterialAsset>
        std::shared_ptr<Engine::Material> GetOrCreate() {
            const std::string key = TMaterialAsset::kName;
            auto it = m_cache.find(key);
            if (it != m_cache.end()) {
                if (auto sp = it->second.lock()) return sp;
            }

            auto mat = TMaterialAsset::Create(m_ctx);
            m_cache[key] = mat;
            return mat;
        }

    private:
        MaterialBuildContext m_ctx{};
        std::unordered_map<std::string, std::weak_ptr<Engine::Material>> m_cache;
    };

} // namespace Game
