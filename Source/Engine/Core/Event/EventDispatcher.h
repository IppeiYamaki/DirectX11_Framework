#pragma once

/// @file   EventDispatcher.h
/// @brief  型安全なイベントディスパッチャー（C++20対応）

#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <any>
#include <concepts>

namespace Engine {

    //============================================================
    // Event Concept
    //============================================================

    /// @brief  イベントとして使用可能な型を制約
    /// @tparam T イベント型
    template<typename T>
    concept EventType = std::is_class_v<T> && std::is_default_initializable<T>;

    //============================================================
    // Event Dispatcher
    //============================================================

    /// @brief  型安全なイベントディスパッチャー
    /// @note   Component間の疎結合通信を実現
    class EventDispatcher final {
    public:
        EventDispatcher() = default;
        ~EventDispatcher() = default;

        EventDispatcher(const EventDispatcher&) = delete;
        EventDispatcher& operator=(const EventDispatcher&) = delete;

        //============================================================
        // Subscribe
        //============================================================

        /// @brief  イベントリスナーを登録
        /// @tparam T イベント型
        /// @param  handler イベントハンドラー
        /// @return 登録ID（後でUnsubscribeに使用）
        template<EventType T>
        std::size_t Subscribe(std::function<void(const T&)> handler) {
            auto typeId = std::type_index(typeid(T));
            std::size_t id = m_nextId++;

            m_handlers[typeId].push_back({
                id,
                [handler](const std::any& event) {
                    handler(std::any_cast<const T&>(event));
                }
            });

            return id;
        }

        /// @brief  イベントリスナーを登録（ラムダ推論版）
        /// @tparam F ハンドラー型（自動推論）
        /// @param  handler イベントハンドラー
        /// @return 登録ID
        template<typename F>
        auto Subscribe(F&& handler) -> std::size_t {
            return Subscribe(std::function(std::forward<F>(handler)));
        }

        //============================================================
        // Unsubscribe
        //============================================================

        /// @brief  イベントリスナーを解除
        /// @tparam T イベント型
        /// @param  id 登録ID
        /// @return 解除に成功したらtrue
        template<EventType T>
        bool Unsubscribe(std::size_t id) {
            auto typeId = std::type_index(typeid(T));
            auto it = m_handlers.find(typeId);
            if (it == m_handlers.end()) return false;

            auto& handlers = it->second;
            for (auto handlerIt = handlers.begin(); handlerIt != handlers.end(); ++handlerIt) {
                if (handlerIt->id == id) {
                    handlers.erase(handlerIt);
                    return true;
                }
            }
            return false;
        }

        //============================================================
        // Dispatch
        //============================================================

        /// @brief  イベントを発行
        /// @tparam T イベント型
        /// @param  event イベントデータ
        template<EventType T>
        void Dispatch(const T& event) {
            auto typeId = std::type_index(typeid(T));
            auto it = m_handlers.find(typeId);
            if (it == m_handlers.end()) return;

            for (auto& handler : it->second) {
                handler.callback(event);
            }
        }

        /// @brief  イベントを発行（rvalue版）
        /// @tparam T イベント型
        /// @param  event イベントデータ
        template<EventType T>
        void Dispatch(T&& event) {
            Dispatch(static_cast<const T&>(event));
        }

        //============================================================
        // Management
        //============================================================

        /// @brief  特定タイプのリスナーをすべてクリア
        /// @tparam T イベント型
        template<EventType T>
        void ClearHandlers() {
            auto typeId = std::type_index(typeid(T));
            m_handlers.erase(typeId);
        }

        /// @brief  全リスナーをクリア
        void Clear() {
            m_handlers.clear();
        }

        /// @brief  登録されているハンドラー数を取得
        /// @tparam T イベント型
        /// @return ハンドラー数
        template<EventType T>
        [[nodiscard]] std::size_t GetHandlerCount() const {
            auto typeId = std::type_index(typeid(T));
            auto it = m_handlers.find(typeId);
            return (it != m_handlers.end()) ? it->second.size() : 0;
        }

    private:
        /// @brief  ハンドラー情報
        struct HandlerInfo {
            std::size_t id;
            std::function<void(const std::any&)> callback;
        };

        std::unordered_map<std::type_index, std::vector<HandlerInfo>> m_handlers;
        std::size_t m_nextId = 0;
    };

    //============================================================
    // Scoped Subscription
    //============================================================

    /// @brief  スコープベースのイベント購読（RAII）
    /// @tparam T イベント型
    template<EventType T>
    class ScopedSubscription final {
    public:
        /// @brief  コンストラクタ
        /// @param  dispatcher ディスパッチャー参照
        /// @param  handler    イベントハンドラー
        ScopedSubscription(EventDispatcher& dispatcher, std::function<void(const T&)> handler)
            : m_dispatcher(&dispatcher)
            , m_id(dispatcher.Subscribe<T>(std::move(handler))) {}

        ~ScopedSubscription() {
            if (m_dispatcher) {
                m_dispatcher->Unsubscribe<T>(m_id);
            }
        }

        ScopedSubscription(const ScopedSubscription&) = delete;
        ScopedSubscription& operator=(const ScopedSubscription&) = delete;

        ScopedSubscription(ScopedSubscription&& other) noexcept
            : m_dispatcher(other.m_dispatcher)
            , m_id(other.m_id) {
            other.m_dispatcher = nullptr;
        }

        ScopedSubscription& operator=(ScopedSubscription&& other) noexcept {
            if (this != &other) {
                if (m_dispatcher) {
                    m_dispatcher->Unsubscribe<T>(m_id);
                }
                m_dispatcher = other.m_dispatcher;
                m_id = other.m_id;
                other.m_dispatcher = nullptr;
            }
            return *this;
        }

    private:
        EventDispatcher* m_dispatcher;
        std::size_t m_id;
    };

} // namespace Engine

//============================================================
// Usage Example
//============================================================
/*
// イベント定義
struct PlayerDamagedEvent {
    Entity* player;
    float damage;
    Entity* attacker;
};

struct PlayerDeathEvent {
    Entity* player;
};

// ディスパッチャー使用例
Engine::EventDispatcher dispatcher;

// 購読
auto id = dispatcher.Subscribe<PlayerDamagedEvent>([](const PlayerDamagedEvent& e) {
    Logger::Info("Player took {} damage", e.damage);
});

// スコープ購読（スコープ終了時に自動解除）
{
    Engine::ScopedSubscription<PlayerDeathEvent> subscription(dispatcher, 
        [](const PlayerDeathEvent& e) {
            Logger::Info("Player died!");
        });
    
    // このスコープ内でのみ有効
}

// イベント発行
dispatcher.Dispatch(PlayerDamagedEvent{ player, 10.0f, enemy });

// 解除
dispatcher.Unsubscribe<PlayerDamagedEvent>(id);
*/
