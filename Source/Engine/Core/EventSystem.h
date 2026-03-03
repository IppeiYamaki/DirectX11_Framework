/// @file   EventSystem.h
/// @brief  型安全なイベントシステム
#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <any>
#include <cstdint>
#include <algorithm>

namespace Engine {

    /// @brief 型安全なイベントシステム
    /// @note  コンポーネント間の疎結合な通信を実現
    class EventSystem final {
    public:
        using EventId = std::string;
        using Handle = std::uint64_t;

        /// @brief シングルトンインスタンスを取得
        /// @return EventSystemインスタンスへの参照
        static EventSystem& GetInstance() {
            static EventSystem instance;
            return instance;
        }

        // コピー/ムーブ禁止（シングルトン）
        EventSystem(const EventSystem&) = delete;
        EventSystem& operator=(const EventSystem&) = delete;
        EventSystem(EventSystem&&) = delete;
        EventSystem& operator=(EventSystem&&) = delete;

        //============================================================
        // 購読/解除
        //============================================================

        /// @brief  イベントを購読
        /// @tparam T        イベントデータ型
        /// @param  eventId  イベント識別子
        /// @param  callback コールバック関数
        /// @return 購読ハンドル（解除に使用）
        template <typename T>
        [[nodiscard]] Handle Subscribe(const EventId& eventId,
                                        std::function<void(const T&)> callback) {
            Handle handle = m_nextHandle++;
            m_callbacks[eventId].push_back({
                handle,
                [callback = std::move(callback)](const std::any& data) {
                    callback(std::any_cast<const T&>(data));
                }
            });
            return handle;
        }

        /// @brief  引数なしイベントを購読
        /// @param  eventId  イベント識別子
        /// @param  callback コールバック関数
        /// @return 購読ハンドル
        [[nodiscard]] Handle Subscribe(const EventId& eventId,
                                        std::function<void()> callback) {
            Handle handle = m_nextHandle++;
            m_callbacks[eventId].push_back({
                handle,
                [callback = std::move(callback)](const std::any&) {
                    callback();
                }
            });
            return handle;
        }

        /// @brief 購読を解除
        /// @param eventId イベント識別子
        /// @param handle  購読ハンドル
        void Unsubscribe(const EventId& eventId, Handle handle) {
            auto it = m_callbacks.find(eventId);
            if (it == m_callbacks.end()) return;

            auto& callbacks = it->second;
            callbacks.erase(
                std::remove_if(callbacks.begin(), callbacks.end(),
                    [handle](const CallbackEntry& entry) {
                        return entry.m_handle == handle;
                    }),
                callbacks.end()
            );

            // コールバックがなくなったら削除
            if (callbacks.empty()) {
                m_callbacks.erase(it);
            }
        }

        /// @brief 特定イベントの全購読を解除
        /// @param eventId イベント識別子
        void UnsubscribeAll(const EventId& eventId) {
            m_callbacks.erase(eventId);
        }

        /// @brief 全イベントの全購読を解除
        void ClearAll() {
            m_callbacks.clear();
        }

        //============================================================
        // 発行
        //============================================================

        /// @brief  イベントを発行（型安全版）
        /// @tparam T       イベントデータ型
        /// @param  eventId イベント識別子
        /// @param  data    イベントデータ
        template <typename T>
        void Publish(const EventId& eventId, const T& data) {
            auto it = m_callbacks.find(eventId);
            if (it == m_callbacks.end()) return;

            // コピーを作成して反復（発行中の購読/解除に対応）
            auto callbacksCopy = it->second;
            for (auto& [handle, callback] : callbacksCopy) {
                callback(std::any(data));
            }
        }

        /// @brief 引数なしイベントを発行
        /// @param eventId イベント識別子
        void Publish(const EventId& eventId) {
            auto it = m_callbacks.find(eventId);
            if (it == m_callbacks.end()) return;

            auto callbacksCopy = it->second;
            for (auto& [handle, callback] : callbacksCopy) {
                callback(std::any{});
            }
        }

        //============================================================
        // クエリ
        //============================================================

        /// @brief イベントに購読者がいるか確認
        /// @param eventId イベント識別子
        /// @return 購読者がいればtrue
        [[nodiscard]] bool HasSubscribers(const EventId& eventId) const {
            auto it = m_callbacks.find(eventId);
            return it != m_callbacks.end() && !it->second.empty();
        }

        /// @brief イベントの購読者数を取得
        /// @param eventId イベント識別子
        /// @return 購読者数
        [[nodiscard]] std::size_t GetSubscriberCount(const EventId& eventId) const {
            auto it = m_callbacks.find(eventId);
            return (it != m_callbacks.end()) ? it->second.size() : 0;
        }

    private:
        EventSystem() = default;
        ~EventSystem() = default;

        using Callback = std::function<void(const std::any&)>;

        struct CallbackEntry {
            Handle m_handle;
            Callback m_callback;
        };

        std::unordered_map<EventId, std::vector<CallbackEntry>> m_callbacks;
        Handle m_nextHandle = 1;
    };

    //============================================================
    // RAII式の購読ガード
    //============================================================

    /// @brief RAII式のイベント購読ガード
    /// @note  スコープを抜けると自動的に購読解除
    class EventSubscriptionGuard final {
    public:
        EventSubscriptionGuard() = default;

        EventSubscriptionGuard(const EventSystem::EventId& eventId,
                               EventSystem::Handle handle)
            : m_eventId(eventId), m_handle(handle), m_isValid(true) {}

        ~EventSubscriptionGuard() {
            Unsubscribe();
        }

        // ムーブのみ許可
        EventSubscriptionGuard(EventSubscriptionGuard&& other) noexcept
            : m_eventId(std::move(other.m_eventId))
            , m_handle(other.m_handle)
            , m_isValid(other.m_isValid) {
            other.m_isValid = false;
        }

        EventSubscriptionGuard& operator=(EventSubscriptionGuard&& other) noexcept {
            if (this != &other) {
                Unsubscribe();
                m_eventId = std::move(other.m_eventId);
                m_handle = other.m_handle;
                m_isValid = other.m_isValid;
                other.m_isValid = false;
            }
            return *this;
        }

        // コピー禁止
        EventSubscriptionGuard(const EventSubscriptionGuard&) = delete;
        EventSubscriptionGuard& operator=(const EventSubscriptionGuard&) = delete;

        /// @brief 手動で購読を解除
        void Unsubscribe() {
            if (m_isValid) {
                EventSystem::GetInstance().Unsubscribe(m_eventId, m_handle);
                m_isValid = false;
            }
        }

        /// @brief 購読が有効か確認
        [[nodiscard]] bool IsValid() const { return m_isValid; }

    private:
        EventSystem::EventId m_eventId;
        EventSystem::Handle m_handle = 0;
        bool m_isValid = false;
    };

    //============================================================
    // ヘルパー関数
    //============================================================

    /// @brief  イベントを購読し、ガードを返す
    /// @tparam T        イベントデータ型
    /// @param  eventId  イベント識別子
    /// @param  callback コールバック関数
    /// @return 購読ガード
    template <typename T>
    [[nodiscard]] EventSubscriptionGuard SubscribeWithGuard(
        const EventSystem::EventId& eventId,
        std::function<void(const T&)> callback) {
        auto handle = EventSystem::GetInstance().Subscribe<T>(eventId, std::move(callback));
        return EventSubscriptionGuard(eventId, handle);
    }

    /// @brief 引数なしイベントを購読し、ガードを返す
    /// @param eventId  イベント識別子
    /// @param callback コールバック関数
    /// @return 購読ガード
    [[nodiscard]] inline EventSubscriptionGuard SubscribeWithGuard(
        const EventSystem::EventId& eventId,
        std::function<void()> callback) {
        auto handle = EventSystem::GetInstance().Subscribe(eventId, std::move(callback));
        return EventSubscriptionGuard(eventId, handle);
    }

} // namespace Engine

// 使用例:
//
// // イベントデータ構造体
// struct DamageEvent {
//     float m_amount;
//     EntityId m_sourceId;
//     EntityId m_targetId;
// };
//
// // 購読（ラムダ式）
// auto handle = EventSystem::GetInstance().Subscribe<DamageEvent>(
//     "OnDamage",
//     [](const DamageEvent& e) {
//         Logger::InfoF("Damage: {}", e.m_amount);
//     }
// );
//
// // 発行
// EventSystem::GetInstance().Publish("OnDamage", DamageEvent{ 10.0f, srcId, tgtId });
//
// // RAII式購読（スコープを抜けると自動解除）
// {
//     auto guard = SubscribeWithGuard<DamageEvent>("OnDamage", handler);
//     // ... スコープ内でのみ有効
// } // 自動的に購読解除
