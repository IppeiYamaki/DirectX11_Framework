# GameObjectアーキテクチャ設計書

このドキュメントは、DirectX11_FrameworkにおけるGameObjectベースのアーキテクチャ設計について説明します。

---

## 目次

1. [概要](#1-概要)
2. [設計方針](#2-設計方針)
3. [クラス構造](#3-クラス構造)
4. [各クラスの責務](#4-各クラスの責務)
5. [使用例](#5-使用例)
6. [ファイル構成](#6-ファイル構成)
7. [テスト計画](#7-テスト計画)

---

## 1. 概要

### 1.1 背景

本フレームワークでは、Unity風の`GameObject + Component`モデルを採用しています。

### 1.2 設計方針

`GameObject`基底クラスを導入し、UnityスタイルのGameObjectパターンを採用：
- すべてのゲームオブジェクトは`GameObject`を継承
- `GameObject`は「器」として設計され、Transform、名前、タグ、コンポーネントリストを保持
- 派生クラスでオブジェクト固有の挙動を自己完結的に記述

---

## 2. 設計方針

### 2.1 基本原則

1. **継承による拡張**: GameObjectを継承して具体的なオブジェクトを実装
2. **コンポーネント合成**: 機能はコンポーネントとして追加可能
3. **ライフサイクル管理**: Initialize → Update → LateUpdate → Render → OnDestroy

### 2.2 Sceneでのオブジェクト管理

```
┌─────────────────────────────────────────────────────────────┐
│                         Scene                                │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │                  GameObject群                            │ │
│  │  - m_gameObjects                                         │ │
│  │  - CreateObject<T>()                                     │ │
│  │  - AddObject()                                           │ │
│  │  - DestroyObject()                                       │ │
│  │  - FindObjectByName() / FindObjectsWithTag()             │ │
│  └─────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

---

## 3. クラス構造

### 3.1 GameObjectクラス階層

```
GameObject (基底クラス)
├── PlayerObject (プレイヤー固有ロジック)
├── EnemyObject (敵固有ロジック + AI)
├── NpcObject (NPC固有ロジック)
├── ItemObject (アイテム固有ロジック)
└── ProjectileObject (弾丸固有ロジック)
```

### 3.2 GameObjectクラス定義

```cpp
class GameObject {
public:
    explicit GameObject(const std::string& name = "");
    virtual ~GameObject();

    // ライフサイクルメソッド（オーバーライド可能）
    virtual void Initialize();
    virtual void Update(float deltaTime);
    virtual void LateUpdate(float deltaTime);
    virtual void Render();
    virtual void OnDestroy();

    // Transform操作
    void SetPosition(const Vector3& position);
    Vector3 GetPosition() const;
    void SetRotation(const Vector3& rotation);
    Vector3 GetRotation() const;
    void SetScale(const Vector3& scale);
    Vector3 GetScale() const;
    Transform* GetTransform();

    // 識別情報
    EntityId GetId() const;
    void SetName(const std::string& name);
    const std::string& GetName() const;
    void SetTag(const std::string& tag);
    const std::string& GetTag() const;
    bool CompareTag(const std::string& tag) const;

    // マルチタグサポート
    void AddTag(const Tag& tag);
    void RemoveTag(const Tag& tag);
    bool HasTag(const Tag& tag) const;
    void ClearTags();

    // 有効/無効制御
    void SetActive(bool active);
    bool IsActive() const;

    // コンポーネント管理
    template <class T, class... Args>
    T* AddComponent(Args&&... args);
    
    template <class T>
    T* GetComponent();
    
    template <class T>
    std::vector<T*> GetComponents();
    
    template <class T>
    bool HasComponent() const;

    // Scene連携
    Scene* GetScene() const;

protected:
    EntityId m_id;
    std::string m_name;
    TagSet m_tags;
    bool m_isActive = true;
    std::vector<std::unique_ptr<Component>> m_components;
    Transform* m_transform = nullptr;
    Scene* m_scene = nullptr;
};
```

### 3.3 派生クラスの例

#### PlayerObject
```cpp
class PlayerObject : public Engine::GameObject {
public:
    explicit PlayerObject(const std::string& name = "Player");

    void Initialize() override;
    void Update(float deltaTime) override;
    void Render() override;
    void OnDestroy() override;

    // プレイヤー固有機能
    void SetMoveSpeed(float speed);
    float GetMoveSpeed() const;
    void SetHealth(float hp);
    float GetHealth() const;
    void TakeDamage(float damage);
    bool IsAlive() const;

protected:
    virtual void HandleInput(float deltaTime);
    void Move(const Engine::Vector3& direction, float deltaTime);

private:
    float m_moveSpeed = 5.0f;
    float m_health = 100.0f;
    float m_maxHealth = 100.0f;
};
```

#### EnemyObject
```cpp
class EnemyObject : public Engine::GameObject {
public:
    enum class State { Idle, Patrol, Chase, Attack, Dead };

    explicit EnemyObject(const std::string& name = "Enemy");

    void Initialize() override;
    void Update(float deltaTime) override;

    // 敵固有機能
    void SetTarget(Engine::GameObject* target);
    void SetDetectionRange(float range);
    void SetAttackRange(float range);
    State GetState() const;

protected:
    virtual void UpdateAI(float deltaTime);
    virtual void OnIdle(float deltaTime);
    virtual void OnPatrol(float deltaTime);
    virtual void OnChase(float deltaTime);
    virtual void OnAttack(float deltaTime);
    virtual void OnDead(float deltaTime);

private:
    float m_moveSpeed = 3.0f;
    float m_health = 50.0f;
    float m_detectionRange = 10.0f;
    float m_attackRange = 2.0f;
    State m_state = State::Idle;
    Engine::GameObject* m_target = nullptr;
};
```

---

## 4. 各クラスの責務

### 4.1 責務分担表

| クラス | 責務 |
|--------|------|
| **Scene** | GameObjectの生成・管理・破棄、Update/Draw統括 |
| **SceneBase** | Sceneステート（状態/空間）のインターフェース定義、共通GameObject管理機能（ObjectSlotCollection使用） |
| **GameObject** | 器として機能、Transform/名前/タグ/コンポーネント保持 |
| **PlayerObject** | プレイヤー固有ロジック（入力処理、HP管理など） |
| **EnemyObject** | 敵固有ロジック（AI、状態遷移など） |
| **Component** | 機能単位の基底クラス、ライフサイクル提供 |
| **Transform** | 位置/回転/スケール管理 |
| **Prefab** | GameObjectの初期設定定義、生成ファクトリ |
| **ObjectSlot** | GameObjectインスタンスの管理（再生成/破棄） |
| **ObjectSlotCollection** | 複数のGameObjectをshared_ptrで管理するコレクション |

### 4.2 依存関係図

```
SceneManager
    │
    └── SceneBase (インターフェース)
            │
            └── SampleScene (具象実装)
                    │
                    └── Scene::CreateObject<T>()
                            │
                            ├── PlayerObject
                            ├── EnemyObject
                            └── (その他のGameObject派生)
                                    │
                                    └── Component群
                                        ├── Transform
                                        ├── MeshRenderer
                                        └── (カスタムComponent)
```

---

## 5. 使用例

### 5.1 シーンでのGameObject生成

```cpp
void SampleScene::BuildScene(Engine::SceneContext& ctx) {
    // Prefab経由でのGameObject生成
    m_cameraObject = ctx.Spawn<MainCameraPrefab>(
        Engine::Vector3(0, 3, -8), 0.0f, -5.0f);

    // 直接GameObjectを生成
    auto* player = ctx.m_scene->CreateObject<PlayerObject>("MainPlayer");
    player->SetPosition(Engine::Vector3(0, 0, 0));
    player->SetMoveSpeed(7.0f);
    player->SetHealth(150.0f);

    // Prefab経由でのGameObject生成
    auto* enemy1 = ctx.Spawn<EnemyPrefab>(
        Engine::Vector3(10, 0, 5), 1.0f, 3.0f, 50.0f);
    
    // 敵にプレイヤーをターゲットとして設定
    if (auto* enemyObj = dynamic_cast<EnemyObject*>(enemy1)) {
        enemyObj->SetTarget(player);
    }
}
```

### 5.2 ObjectSlotを使った管理

```cpp
class GameplayScene : public Engine::SceneBase {
private:
    Engine::ObjectSlot m_playerSlot;
    std::vector<Engine::ObjectSlot> m_enemySlots;

public:
    void Initialize(Engine::SceneContext& ctx) override {
        // プレイヤー生成
        m_playerSlot.Spawn<PlayerPrefab>(ctx, 
            Engine::Vector3(0, 0, 0), 1.0f, 5.0f, 100.0f);

        // 敵を複数生成
        for (int i = 0; i < 5; ++i) {
            m_enemySlots.emplace_back();
            m_enemySlots.back().Spawn<EnemyPrefab>(ctx,
                Engine::Vector3(i * 5.0f, 0, 10.0f));
        }
    }

    void Finalize(Engine::SceneContext& ctx) override {
        m_playerSlot.Destroy(ctx);
        for (auto& slot : m_enemySlots) {
            slot.Destroy(ctx);
        }
        m_enemySlots.clear();
    }
};
```

### 5.3 派生クラスでのカスタマイズ

```cpp
class BossEnemy : public Game::EnemyObject {
public:
    explicit BossEnemy(const std::string& name = "Boss")
        : EnemyObject(name) {
        SetHealth(500.0f);
        SetMoveSpeed(2.0f);
        SetDetectionRange(20.0f);
    }

    void Initialize() override {
        EnemyObject::Initialize();
        
        // ボス固有のコンポーネント追加
        AddComponent<BossHealthBar>();
        AddComponent<SpecialAttackController>();
    }

protected:
    void OnAttack(float deltaTime) override {
        // ボス固有の攻撃ロジック
        if (m_specialAttackCooldown <= 0.0f) {
            PerformSpecialAttack();
            m_specialAttackCooldown = 10.0f;
        } else {
            EnemyObject::OnAttack(deltaTime);
        }
        m_specialAttackCooldown -= deltaTime;
    }

private:
    float m_specialAttackCooldown = 0.0f;
    
    void PerformSpecialAttack() {
        // 特殊攻撃の実装
    }
};
```

---

## 6. ファイル構成

### 6.1 ディレクトリ構造

```
Source/
├── Engine/
│   └── Scene/
│       ├── Component.h/cpp          # コンポーネント基底
│       ├── EntityId.h               # ID/タグシステム
│       ├── GameObject.h/cpp         # GameObject基底クラス
│       ├── ObjectSlot.h             # GameObject管理用Slot（Engine共通）
│       ├── Scene.h/cpp              # GameObject管理
│       ├── SceneBase.h              # Sceneインターフェース
│       ├── SceneContext.h           # Sceneコンテキスト
│       ├── SceneManager.h/cpp       # Scene切替管理
│       └── Components/
│           ├── Transform.h/cpp
│           ├── MeshRenderer.h/cpp
│           └── ...
│
└── Game/
    ├── GameObjects/                  # GameObject派生クラス
    │   ├── PlayerObject.h/cpp
    │   ├── EnemyObject.h/cpp
    │   └── ...
    │
    ├── Prefabs/
    │   ├── PlayerPrefab.h/cpp        # GameObjectベースPrefab
    │   ├── EnemyPrefab.h/cpp         # GameObjectベースPrefab
    │   ├── SamplePrefab.h/cpp        # GameObjectベース
    │   └── ...
    │
    └── Scenes/
        ├── SampleScene.h/cpp
        └── ...
```

### 6.2 主要ファイル一覧

| ファイル | 説明 |
|----------|------|
| `Engine/Scene/GameObject.h` | GameObject基底クラスヘッダ |
| `Engine/Scene/GameObject.cpp` | GameObject基底クラス実装 |
| `Engine/Scene/ObjectSlot.h` | GameObjectスロット管理（Engine共通） |
| `Game/GameObjects/PlayerObject.h` | プレイヤーオブジェクトヘッダ |
| `Game/GameObjects/PlayerObject.cpp` | プレイヤーオブジェクト実装 |
| `Game/GameObjects/EnemyObject.h` | 敵オブジェクトヘッダ |
| `Game/GameObjects/EnemyObject.cpp` | 敵オブジェクト実装 |
| `Game/Prefabs/PlayerPrefab.h` | プレイヤーPrefabヘッダ |
| `Game/Prefabs/PlayerPrefab.cpp` | プレイヤーPrefab実装 |
| `Game/Prefabs/EnemyPrefab.h` | 敵Prefabヘッダ |
| `Game/Prefabs/EnemyPrefab.cpp` | 敵Prefab実装 |

---

## 7. テスト計画

### 7.1 単体テスト項目

#### GameObject基本機能テスト
```cpp
// テスト1: 生成と初期化
void Test_GameObject_CreateAndInitialize() {
    Engine::Scene scene;
    scene.Initialize();
    
    auto* obj = scene.CreateObject<Engine::GameObject>("TestObject");
    
    ASSERT_NE(obj, nullptr);
    ASSERT_EQ(obj->GetName(), "TestObject");
    ASSERT_TRUE(obj->IsActive());
    ASSERT_NE(obj->GetTransform(), nullptr);
}

// テスト2: Transform操作
void Test_GameObject_TransformOperations() {
    Engine::Scene scene;
    scene.Initialize();
    
    auto* obj = scene.CreateObject<Engine::GameObject>("Test");
    
    obj->SetPosition(Engine::Vector3(1.0f, 2.0f, 3.0f));
    ASSERT_EQ(obj->GetPosition().x, 1.0f);
    ASSERT_EQ(obj->GetPosition().y, 2.0f);
    ASSERT_EQ(obj->GetPosition().z, 3.0f);
    
    obj->SetScale(Engine::Vector3(2.0f, 2.0f, 2.0f));
    ASSERT_EQ(obj->GetScale().x, 2.0f);
}

// テスト3: コンポーネント追加/取得
void Test_GameObject_ComponentManagement() {
    Engine::Scene scene;
    scene.Initialize();
    
    auto* obj = scene.CreateObject<Engine::GameObject>("Test");
    
    // Transformは自動追加される
    ASSERT_TRUE(obj->HasComponent<Engine::Transform>());
    
    // カスタムコンポーネント追加
    auto* rotator = obj->AddComponent<Game::SampleRotateComponent>(45.0f);
    ASSERT_NE(rotator, nullptr);
    ASSERT_TRUE(obj->HasComponent<Game::SampleRotateComponent>());
}

// テスト4: 有効/無効制御
void Test_GameObject_ActiveControl() {
    Engine::Scene scene;
    scene.Initialize();
    
    auto* obj = scene.CreateObject<Engine::GameObject>("Test");
    ASSERT_TRUE(obj->IsActive());
    
    obj->SetActive(false);
    ASSERT_FALSE(obj->IsActive());
    
    obj->SetActive(true);
    ASSERT_TRUE(obj->IsActive());
}
```

### 7.2 テスト実行方法

1. **コンパイル確認**: すべてのファイルがコンパイルできることを確認
2. **基本動作確認**: サンプルシーンでGameObjectが正しく生成・更新・描画されることを確認
3. **メモリリーク確認**: オブジェクトの生成・破棄でメモリリークがないことを確認

---

## 付録

### A. 将来の拡張予定

- [ ] 親子階層（GameObject間の親子関係）
- [ ] プーリングシステム（オブジェクトの再利用）
- [ ] シリアライズ/デシリアライズ（セーブ/ロード対応）
- [ ] イベントシステム統合
