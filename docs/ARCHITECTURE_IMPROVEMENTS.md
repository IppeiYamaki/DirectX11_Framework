# Entity + Component アーキテクチャ改善提案

## 概要

現在のUnity風Entity+Componentモデルを分析し、パフォーマンス・拡張性・保守性を向上させる改善案を提示します。

---

## 現状分析

### 良い点 ✅

1. **明確な責務分離**: Entity、Component、Worldが適切に分離されている
2. **UnityライクなAPI**: `AddComponent<T>()`, `GetComponent<T>()` など直感的
3. **ライフサイクル管理**: OnAwake, OnStart, Update, LateUpdate, Draw, OnDestroyが整備
4. **カプセル化**: ComponentのownerはEntity経由でのみ設定可能

### 改善可能な点 🔧

1. **メモリ局所性**: `vector<unique_ptr<Component>>` はキャッシュ効率が悪い
2. **動的型チェック**: `dynamic_cast` による毎フレームの型判定コスト
3. **遅延破棄なし**: Update中のDestroyEntityがイテレーター破壊の危険
4. **Entity検索**: 特定タグ/名前のEntity検索機能がない
5. **Component間通信**: 直接参照のみ、イベントシステムがない

---

## 改善提案

### 1. 遅延破棄システム（高優先度）

**問題:**
```cpp
void World::DestroyEntity(Entity* entity) {
    // Update中に呼ばれるとイテレーター破壊
    for (auto it = m_entities.begin(); it != m_entities.end(); ++it) {
        if (it->get() == entity) {
            m_entities.erase(it); // 危険！
            return;
        }
    }
}
```

**解決案:**
```cpp
// World.h に追加
class World final {
public:
    void DestroyEntity(Entity* entity);   // 破棄予約
    void DestroyEntityImmediate(Entity* entity);  // 即時破棄（危険）

private:
    void ProcessPendingDestruction();      // フレーム終了時に実行

private:
    std::vector<std::unique_ptr<Entity>> m_entities;
    std::vector<Entity*> m_pendingDestruction;  // 破棄予約リスト
    bool m_isUpdating = false;                  // 更新中フラグ
};
```

```cpp
// World.cpp
void World::DestroyEntity(Entity* entity) {
    if (!m_isInitialized || entity == nullptr) return;

    if (m_isUpdating) {
        // 更新中なら予約
        m_pendingDestruction.push_back(entity);
    } else {
        // 更新中でなければ即時削除
        DestroyEntityImmediate(entity);
    }
}

void World::ProcessPendingDestruction() {
    for (Entity* e : m_pendingDestruction) {
        DestroyEntityImmediate(e);
    }
    m_pendingDestruction.clear();
}

void World::Update(float deltaTime) {
    if (!m_isInitialized) return;

    m_isUpdating = true;

    for (auto& e : m_entities) {
        e->UpdateComponents(deltaTime);
    }

    m_isUpdating = false;
    ProcessPendingDestruction();  // 予約された破棄を処理
}
```

---

### 2. ComponentPool による最適化（中優先度）

**目的:** メモリ局所性を向上させ、キャッシュ効率を改善

```cpp
// Engine/Scene/ComponentPool.h
#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

namespace Engine {

    /// @brief 型ごとのComponent連続メモリプール
    /// @tparam T Component派生型
    template<typename T>
    class ComponentPool final {
    public:
        ComponentPool() = default;
        ~ComponentPool() = default;

        ComponentPool(const ComponentPool&) = delete;
        ComponentPool& operator=(const ComponentPool&) = delete;

        /// @brief Componentを生成して返す
        template<typename... Args>
        T* Create(Args&&... args) {
            m_components.emplace_back(std::forward<Args>(args)...);
            T* raw = &m_components.back();
            m_entityMap[raw->GetOwner()] = raw;
            return raw;
        }

        /// @brief 指定Entityに紐づくComponentを取得
        T* Get(Entity* entity) {
            auto it = m_entityMap.find(entity);
            return (it != m_entityMap.end()) ? it->second : nullptr;
        }

        /// @brief 全Componentを更新
        void UpdateAll(float deltaTime) {
            for (auto& comp : m_components) {
                if (comp.IsEnabled()) {
                    comp.Update(deltaTime);
                }
            }
        }

        /// @brief 全Componentを描画
        void DrawAll() {
            for (auto& comp : m_components) {
                if (comp.IsEnabled()) {
                    comp.Draw();
                }
            }
        }

        /// @brief Component数を返す
        [[nodiscard]] size_t Size() const noexcept {
            return m_components.size();
        }

    private:
        std::vector<T> m_components;                        // 連続メモリ
        std::unordered_map<Entity*, T*> m_entityMap;        // Entity→Component検索
    };

} // namespace Engine
```

**使用例:**
```cpp
// System側での使用
class TransformSystem {
public:
    void Update(float deltaTime) {
        // 連続メモリでキャッシュフレンドリー
        for (auto& transform : m_pool) {
            transform.RebuildWorldIfDirty();
        }
    }

private:
    ComponentPool<Transform> m_pool;
};
```

---

### 3. タグシステム（中優先度）

**目的:** Entity検索の効率化

```cpp
// Engine/Scene/Entity.h に追加
class Entity final {
public:
    // タグ操作
    void AddTag(const std::string& tag);
    void RemoveTag(const std::string& tag);
    bool HasTag(const std::string& tag) const;
    const std::unordered_set<std::string>& GetTags() const;

private:
    std::unordered_set<std::string> m_tags;
};

// Engine/Scene/World.h に追加
class World final {
public:
    /// @brief タグでEntityを検索
    Entity* FindEntityByTag(const std::string& tag);
    
    /// @brief タグで全Entityを検索
    std::vector<Entity*> FindEntitiesByTag(const std::string& tag);

private:
    std::unordered_multimap<std::string, Entity*> m_tagIndex;  // タグ→Entity索引
};
```

---

### 4. イベントシステム（中優先度）

**目的:** Component間の疎結合通信

```cpp
// Engine/Core/Event.h
#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <any>

namespace Engine {

    /// @brief イベント基底クラス
    struct EventBase {
        virtual ~EventBase() = default;
    };

    /// @brief イベントディスパッチャー
    class EventDispatcher final {
    public:
        EventDispatcher() = default;
        ~EventDispatcher() = default;

        EventDispatcher(const EventDispatcher&) = delete;
        EventDispatcher& operator=(const EventDispatcher&) = delete;

        /// @brief イベントリスナーを登録
        template<typename T>
        void Subscribe(std::function<void(const T&)> handler) {
            static_assert(std::is_base_of_v<EventBase, T>, "T must derive from EventBase");
            auto typeId = std::type_index(typeid(T));
            m_handlers[typeId].push_back([handler](const std::any& event) {
                handler(std::any_cast<const T&>(event));
            });
        }

        /// @brief イベントを発行
        template<typename T>
        void Dispatch(const T& event) {
            static_assert(std::is_base_of_v<EventBase, T>, "T must derive from EventBase");
            auto typeId = std::type_index(typeid(T));
            auto it = m_handlers.find(typeId);
            if (it != m_handlers.end()) {
                for (auto& handler : it->second) {
                    handler(event);
                }
            }
        }

        /// @brief 全リスナーをクリア
        void Clear() {
            m_handlers.clear();
        }

    private:
        using HandlerFunc = std::function<void(const std::any&)>;
        std::unordered_map<std::type_index, std::vector<HandlerFunc>> m_handlers;
    };

} // namespace Engine
```

**使用例:**
```cpp
// イベント定義
struct PlayerDamagedEvent : EventBase {
    Entity* player;
    float damage;
};

// リスナー登録
world.GetEventDispatcher().Subscribe<PlayerDamagedEvent>([](const auto& e) {
    Logger::Info("Player damaged: {}", e.damage);
});

// イベント発行
world.GetEventDispatcher().Dispatch(PlayerDamagedEvent{player, 10.0f});
```

---

### 5. ステートパターン導入（中優先度）

**目的:** ゲームステート管理の整理

```cpp
// Engine/Core/State.h
#pragma once

namespace Engine {

    /// @brief ステート基底クラス
    /// @tparam Context ステートが操作するコンテキスト型
    template<typename Context>
    class State {
    public:
        virtual ~State() = default;

        /// @brief ステート開始時
        virtual void OnEnter(Context& context) {}
        
        /// @brief ステート終了時
        virtual void OnExit(Context& context) {}
        
        /// @brief 毎フレーム更新
        virtual void Update(Context& context, float deltaTime) {}
        
        /// @brief 毎フレーム描画
        virtual void Draw(Context& context) {}
    };

    /// @brief ステートマシン
    template<typename Context>
    class StateMachine final {
    public:
        explicit StateMachine(Context& context)
            : m_context(context) {}

        ~StateMachine() = default;

        StateMachine(const StateMachine&) = delete;
        StateMachine& operator=(const StateMachine&) = delete;

        /// @brief ステートを変更
        void ChangeState(std::unique_ptr<State<Context>> newState) {
            m_nextState = std::move(newState);
        }

        /// @brief 毎フレーム更新
        void Update(float deltaTime) {
            ApplyPendingStateChange();
            if (m_currentState) {
                m_currentState->Update(m_context, deltaTime);
            }
        }

        /// @brief 毎フレーム描画
        void Draw() {
            if (m_currentState) {
                m_currentState->Draw(m_context);
            }
        }

        /// @brief 現在のステートを取得
        [[nodiscard]] State<Context>* GetCurrentState() const {
            return m_currentState.get();
        }

    private:
        void ApplyPendingStateChange() {
            if (m_nextState) {
                if (m_currentState) {
                    m_currentState->OnExit(m_context);
                }
                m_currentState = std::move(m_nextState);
                m_currentState->OnEnter(m_context);
            }
        }

    private:
        Context& m_context;
        std::unique_ptr<State<Context>> m_currentState;
        std::unique_ptr<State<Context>> m_nextState;
    };

} // namespace Engine
```

---

### 6. 親子関係（Transform階層）

**目的:** 階層構造でのTransform伝播

```cpp
// Transform.h に追加
class Transform final : public Component {
public:
    // 親子関係
    void SetParent(Transform* parent);
    Transform* GetParent() const;
    
    void AddChild(Transform* child);
    void RemoveChild(Transform* child);
    const std::vector<Transform*>& GetChildren() const;

    // ワールド座標取得
    Vector3 GetWorldPosition() const;
    Vector3 GetWorldScale() const;
    DirectX::XMFLOAT4X4 GetWorldMatrix() const;

    // ローカル座標取得
    const Vector3& GetLocalPosition() const;
    const Vector3& GetLocalScale() const;

private:
    void UpdateWorldMatrix() const;
    void MarkChildrenDirty();

private:
    Transform* m_parent = nullptr;
    std::vector<Transform*> m_children;

    Vector3 m_localPosition{ 0.0f, 0.0f, 0.0f };
    Vector3 m_localRotation{ 0.0f, 0.0f, 0.0f };
    Vector3 m_localScale{ 1.0f, 1.0f, 1.0f };

    mutable bool m_isDirty = true;
    mutable DirectX::XMFLOAT4X4 m_worldMatrix{};
};
```

---

## 実装順序の推奨

| 順序 | 機能 | 優先度 | 工数 | 依存関係 |
|------|------|--------|------|----------|
| 1 | 遅延破棄システム | 高 | 2日 | なし |
| 2 | タグシステム | 中 | 2日 | なし |
| 3 | イベントシステム | 中 | 3日 | なし |
| 4 | Transform親子関係 | 中 | 3日 | なし |
| 5 | ComponentPool | 中 | 1週間 | 1 |
| 6 | ステートパターン | 中 | 2日 | なし |

---

## 将来の拡張（ECS移行）

長期的には、より本格的なECS（Entity Component System）への移行も検討できます：

```
[現状: Unity風 OOP ECS]
Entity owns Components
└── 直感的だが、パフォーマンス限界あり

[将来: Data-Oriented ECS]
World manages:
├── EntityRegistry (ID only)
├── ComponentStorage<T> (sparse set or archetype)
└── Systems (ロジック)

メリット:
- 最大のキャッシュ効率
- 並列処理対応
- データ指向設計
```

---

## 次のステップ

1. [ROADMAP.md](./ROADMAP.md) で全体計画を確認
2. 遅延破棄システムから実装開始
3. 単体テストで動作確認
4. 順次追加機能を実装
