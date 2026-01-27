/// @file   Entity.h
/// @brief  Componentの器となるEntityクラス
#pragma once

#include <memory>
#include <type_traits>
#include <vector>
#include <string>

#include "Engine/Scene/Component.h"
#include "Engine/Scene/EntityId.h"

namespace Engine {

    /// @brief Componentの器（GameObject相当）
    /// @note  Add/Get/Remove操作、enabled制御、ライフサイクルを担当
    class Entity final {
    public:
        Entity();
        ~Entity();

        Entity(const Entity&) = delete;
        Entity& operator=(const Entity&) = delete;

        //============================================================
        // Identification
        //============================================================

        /// @brief  エンティティIDを取得
        /// @return エンティティの一意識別子
        [[nodiscard]] EntityId GetId() const;

        /// @brief 名前を設定
        /// @param name エンティティ名
        void SetName(const std::string& name);

        /// @brief  名前を取得
        /// @return エンティティ名への参照
        [[nodiscard]] const std::string& GetName() const;

        /// @brief タグを追加
        /// @param tag 追加するタグ
        void AddTag(const Tag& tag);

        /// @brief タグを削除
        /// @param tag 削除するタグ
        void RemoveTag(const Tag& tag);

        /// @brief  タグを持っているか確認
        /// @param  tag チェックするタグ
        /// @return タグを持っていればtrue
        [[nodiscard]] bool HasTag(const Tag& tag) const;

        //============================================================
        // Enable
        //============================================================

        /// @brief エンティティを有効化
        void Enable();

        /// @brief エンティティを無効化
        void Disable();

        /// @brief  エンティティが有効か確認
        /// @return 有効ならtrue
        [[nodiscard]] bool IsEnabled() const;

        //============================================================
        // Component
        //============================================================

        /// @brief  Componentを追加して返す（所有はEntity）
        /// @tparam T    Componentを継承した型
        /// @tparam Args コンストラクタ引数型
        /// @param  args コンストラクタ引数
        /// @return 追加されたComponentへのポインタ
        template <class T, class... Args>
        [[nodiscard]] T* AddComponent(Args&&... args) {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

            auto comp = std::make_unique<T>(std::forward<Args>(args)...);
            T* raw = comp.get();

            raw->SetOwner(this);
            m_components.emplace_back(std::move(comp));

            raw->OnAwake();

            if (m_hasStarted) {
                raw->OnStart();
            }

            return raw;
        }

        /// @brief  最初に見つかったT型Componentを返す
        /// @tparam T 検索するComponent型
        /// @return Componentへのポインタ（見つからなければnullptr）
        template <class T>
        [[nodiscard]] T* GetComponent() {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            for (auto& c : m_components) {
                if (auto* t = dynamic_cast<T*>(c.get())) {
                    return t;
                }
            }
            return nullptr;
        }

        /// @brief  最初に見つかったT型Componentを返す（const版）
        /// @tparam T 検索するComponent型
        /// @return Componentへのconstポインタ
        template <class T>
        [[nodiscard]] const T* GetComponent() const {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            for (const auto& c : m_components) {
                if (auto* t = dynamic_cast<const T*>(c.get())) {
                    return t;
                }
            }
            return nullptr;
        }

        /// @brief  全てのT型Componentを取得
        /// @tparam T 検索するComponent型
        /// @return Componentのベクター
        template <class T>
        [[nodiscard]] std::vector<T*> GetComponents() {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            std::vector<T*> result;
            for (auto& c : m_components) {
                if (auto* t = dynamic_cast<T*>(c.get())) {
                    result.push_back(t);
                }
            }
            return result;
        }

        /// @brief  T型のComponentを全て削除
        /// @tparam T 削除するComponent型
        /// @return 削除した数
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

        /// @brief  T型のComponentを持っているか確認
        /// @tparam T 検索するComponent型
        /// @return 持っていればtrue
        template <class T>
        [[nodiscard]] bool HasComponent() const {
            return GetComponent<T>() != nullptr;
        }

        /// @brief  Componentの数を取得
        /// @return Component数
        [[nodiscard]] std::size_t GetComponentCount() const {
            return m_components.size();
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
        EntityIdentity m_identity;                              ///< ID/名前/タグ
        bool m_isEnabled  = true;                               ///< 有効フラグ
        bool m_hasStarted = false;                              ///< Start済みフラグ

        std::vector<std::unique_ptr<Component>> m_components;   ///< 所有Component群
    };

} // namespace Engine
