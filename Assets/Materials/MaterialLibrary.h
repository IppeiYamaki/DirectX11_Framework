#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <type_traits>

#include "Materials/MaterialBuildContext.h"

namespace Engine { class Material; }

namespace Game {

    /**
     * @brief “Material資産” を生成してキャッシュする
     * - SampleCubeMaterial などの “Materialクラス(h/cpp)” をここで使い回す
     *
     * ルール：
     * - TMaterialAsset は `static constexpr const char* kName` を持つこと
     * - `static std::shared_ptr<Engine::Material> Create(const MaterialBuildContext&)` を持つこと
     */
    class MaterialLibrary final {
    public:
        MaterialLibrary() = default;
        ~MaterialLibrary() = default;

        void Initialize(const MaterialBuildContext& ctx);
        void Finalize();

        template<class TMaterialAsset>
        std::shared_ptr<Engine::Material> GetOrCreate() {
            // ★kNameを必須にして、分かりやすいエラーメッセージにする
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

} // namespace Game
