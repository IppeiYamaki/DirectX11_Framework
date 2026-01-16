#pragma once

#include <memory>
#include <type_traits>
#include <vector>

#include "Engine/Scene/Component.h"

namespace Engine {

    /**
     * @brief Componentの器（GameObject相当）
     *
     * - Add/Get/Remove を提供
     * - enabled を持つ
     * - ライフサイクル（OnAwake/OnStart/Update/LateUpdate/Draw/OnDestroy）を内部で伝播
     */
    class Entity final {
    public:
        Entity();
        ~Entity();

        Entity(const Entity&) = delete;
        Entity& operator=(const Entity&) = delete;

        //============================================================
        // Enable
        //============================================================
        void Enable();
        void Disable();
        bool IsEnabled() const;

        //============================================================
        // Component
        //============================================================

        /**
         * @brief Componentを追加して返す（所有はEntity）
         */
        template <class T, class... Args>
        T* AddComponent(Args&&... args) {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

            auto comp = std::make_unique<T>(std::forward<Args>(args)...);
            T* raw = comp.get();

            // owner設定（外部から変更不可）
            raw->SetOwner(this);

            m_components.emplace_back(std::move(comp));

            // 追加時に Awake を呼ぶ（Unity風）
            raw->OnAwake();

            // 既にStart済みなら、新規追加分もStart
            if (m_hasStarted) {
                raw->OnStart();
            }

            return raw;
        }

        /**
         * @brief 最初に見つかった T を返す（無ければ nullptr）
         */
        template <class T>
        T* GetComponent() {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            for (auto& c : m_components) {
                if (auto* t = dynamic_cast<T*>(c.get())) {
                    return t;
                }
            }
            return nullptr;
        }

        template <class T>
        const T* GetComponent() const {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            for (const auto& c : m_components) {
                if (auto* t = dynamic_cast<const T*>(c.get())) {
                    return t;
                }
            }
            return nullptr;
        }

        /**
         * @brief T を全削除（1個だけにしたいなら後で RemoveComponentFirst<T>() を追加）
         * @return 削除した個数
         */
        template <class T>
        int RemoveComponents() {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

            int removed = 0;
            for (auto it = m_components.begin(); it != m_components.end(); ) {
                if (dynamic_cast<T*>(it->get()) != nullptr) {
                    (*it)->OnDestroy();
                    it = m_components.erase(it);
                    ++removed;
                }
                else {
                    ++it;
                }
            }
            return removed;
        }

        //============================================================
        // Internal (called by World)
        //============================================================
        void StartIfNeeded();
        void UpdateComponents(float deltaTime);
        void LateUpdateComponents(float deltaTime);
        void DrawComponents();
        void DestroyComponents();

    private:
        bool m_isEnabled    = true;    // 有効化フラグ
		bool m_hasStarted   = false;   // Start済みフラグ

		std::vector<std::unique_ptr<Component>> m_components;   // 所有コンポーネント群
    };

} // namespace Engine
