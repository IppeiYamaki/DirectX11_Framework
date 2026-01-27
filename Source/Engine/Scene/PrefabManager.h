/// @file   PrefabManager.h
/// @brief  Prefabの生成とキャッシュ管理
#pragma once

#include <memory>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "Engine/Scene/Prefab.h"

namespace Engine {

    /**
     * @brief Prefabのキャッシュ管理
     * @note  同一型のPrefabは1つだけ生成し、参照を返す
     */
    class PrefabManager final {
    public:
        PrefabManager() = default;
        ~PrefabManager() = default;

        PrefabManager(const PrefabManager&) = delete;
        PrefabManager& operator=(const PrefabManager&) = delete;

        /// @brief Prefabを取得（必要なら生成してキャッシュ）
        /// @tparam TPrefab Prefab派生型
        /// @tparam Args Prefab生成に必要な引数
        /// @param args Prefabのコンストラクタ引数
        /// @return Prefabインスタンスへの参照
        template<class TPrefab, class... Args>
        TPrefab& GetOrCreate(Args&&... args) {
            static_assert(std::is_base_of_v<PrefabBase, TPrefab>, "TPrefab must derive from PrefabBase");

            const std::size_t typeKey = typeid(TPrefab).hash_code();
            auto it = m_prefabs.find(typeKey);
            if (it != m_prefabs.end()) {
                return *static_cast<TPrefab*>(it->second.get());
            }

            auto prefab = std::make_unique<TPrefab>(std::forward<Args>(args)...);
            TPrefab& ref = *prefab;
            m_prefabs.emplace(typeKey, std::move(prefab));
            return ref;
        }

        /// @brief Prefabキャッシュをクリア
        void Clear() { m_prefabs.clear(); }

    private:
        std::unordered_map<std::size_t, std::unique_ptr<PrefabBase>> m_prefabs;
    };

} // namespace Engine
