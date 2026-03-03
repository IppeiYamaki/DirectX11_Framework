#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <type_traits>

#include "Engine/Materials/MaterialBuildContext.h"

namespace Engine { class Material; }

namespace Engine {

    /// 
    class MaterialLibrary final {
    public:
        MaterialLibrary() = default;
        ~MaterialLibrary() = default;

        void Initialize(const MaterialBuildContext& ctx);
        void Finalize();

        template<class TMaterialAsset>
        std::shared_ptr<Engine::Material> GetOrCreate() {
            // 
            static_assert(std::is_convertible_v<decltype(TMaterialAsset::kName), const char*>,
                "TMaterialAsset must define: static constexpr const char* kName");

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

} // namespace Engine
