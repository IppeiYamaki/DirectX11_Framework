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

従来のフレームワークでは、`Entity + Component`モデルを採用していましたが、以下の課題がありました：
- Sceneで生成されるオブジェクトに統一的な基底クラスがない
- 各オブジェクトの責務（Transform情報や挙動など）が分散している
- オブジェクト固有のロジックを記述する場所が明確でない

### 1.2 解決策

`GameObject`基底クラスを導入し、UnityスタイルのGameObjectパターンを採用します：
- すべてのゲームオブジェクトは`GameObject`を継承
- `GameObject`は「器」として設計され、Transform、名前、タグ、コンポーネントリストを保持
- 派生クラスでオブジェクト固有の挙動を自己完結的に記述

---

## 2. 設計方針

### 2.1 基本原則

1. **継承による拡張**: GameObjectを継承して具体的なオブジェクトを実装
2. **コンポーネント合成**: 機能はコンポーネントとして追加可能
3. **ライフサイクル管理**: Initialize → Update → LateUpdate → Render → OnDestroy
4. **後方互換性**: 既存のEntity + Componentモデルと共存

### 2.2 既存モデルとの関係

```
┌─────────────────────────────────────────────────────────────┐
│                         Scene                                │
│  ┌─────────────────────┐    ┌─────────────────────────────┐ │
│  │  Entity群（既存）    │    │  GameObject群（新規）        │ │
│  │  - m_entities       │    │  - m_gameObjects            │ │
│  │  - CreateEntity()   │    │  - CreateObject<T>()        │ │
│  │  - DestroyEntity()  │    │  - AddObject()              │ │
│  │                     │    │  - DestroyObject()          │ │
│  └─────────────────────┘    └─────────────────────────────┘ │
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
    std::string m_tag;
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
| **Scene** | Entity/GameObjectの生成・管理・破棄、Update/Draw統括 |
| **SceneBase** | Sceneステート（状態/空間）のインターフェース定義 |
| **GameObject** | 器として機能、Transform/名前/タグ/コンポーネント保持 |
| **PlayerObject** | プレイヤー固有ロジック（入力処理、HP管理など） |
| **EnemyObject** | 敵固有ロジック（AI、状態遷移など） |
| **Component** | 機能単位の基底クラス、ライフサイクル提供 |
| **Transform** | 位置/回転/スケール管理 |
| **Prefab** | GameObjectの初期設定定義、生成ファクトリ |
| **PrefabSlot/GameObjectSlot** | Prefabインスタンスの管理（再生成/破棄） |

### 4.2 依存関係図

```
SceneManager
    │
    └── SceneBase (インターフェース)
            │
            └── SampleScene (具象実装)
                    │
                    ├── Scene::CreateEntity() [既存]
                    └── Scene::CreateObject<T>() [新規]
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
    // 既存のEntity方式（互換性維持）
    m_cameraEntity = ctx.Spawn<MainCameraPrefab>(
        Engine::Vector3(0, 3, -8), 0.0f, -5.0f);

    // 新規のGameObject方式
    auto* player = ctx.m_scene->CreateObject<PlayerObject>("MainPlayer");
    player->SetPosition(Engine::Vector3(0, 0, 0));
    player->SetMoveSpeed(7.0f);
    player->SetHealth(150.0f);

    // Prefab経由でのGameObject生成
    auto* enemy1 = ctx.SpawnObject<EnemyPrefab>(
        Engine::Vector3(10, 0, 5), 1.0f, 3.0f, 50.0f);
    
    // 敵にプレイヤーをターゲットとして設定
    if (auto* enemyObj = dynamic_cast<EnemyObject*>(enemy1)) {
        enemyObj->SetTarget(player);
    }
}
```

### 5.2 GameObjectSlotを使った管理

```cpp
class GameplayScene : public Engine::SceneBase {
private:
    Game::GameObjectSlot m_playerSlot;
    std::vector<Game::GameObjectSlot> m_enemySlots;

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
│       ├── Entity.h/cpp             # Entity（既存互換）
│       ├── EntityId.h               # ID/タグシステム
│       ├── GameObject.h/cpp         # ★ GameObject基底クラス
│       ├── Scene.h/cpp              # Entity/GameObject管理
│       ├── SceneBase.h              # Sceneインターフェース
│       ├── SceneContext.h           # Sceneコンテキスト
│       ├── SceneManager.h/cpp       # Scene切替管理
│       └── Components/
│           ├── Transform.h/cpp
│           ├── MeshRenderer.h/cpp
│           └── ...
│
└── Game/
    ├── GameObjects/                  # ★ GameObject派生クラス
    │   ├── PlayerObject.h/cpp
    │   ├── EnemyObject.h/cpp
    │   └── ...
    │
    ├── Prefabs/
    │   ├── PlayerPrefab.h/cpp        # ★ GameObjectベースPrefab
    │   ├── EnemyPrefab.h/cpp         # ★ GameObjectベースPrefab
    │   ├── SamplePrefab.h/cpp        # 既存（Entityベース）
    │   └── ...
    │
    └── Scenes/
        ├── GameObjectSlot.h          # ★ GameObject管理用Slot
        ├── PrefabSlot.h              # 既存（Entityベース）
        ├── SampleScene.h/cpp
        └── ...
```

### 6.2 新規追加ファイル一覧

| ファイル | 説明 |
|----------|------|
| `Engine/Scene/GameObject.h` | GameObject基底クラスヘッダ |
| `Engine/Scene/GameObject.cpp` | GameObject基底クラス実装 |
| `Game/GameObjects/PlayerObject.h` | プレイヤーオブジェクトヘッダ |
| `Game/GameObjects/PlayerObject.cpp` | プレイヤーオブジェクト実装 |
| `Game/GameObjects/EnemyObject.h` | 敵オブジェクトヘッダ |
| `Game/GameObjects/EnemyObject.cpp` | 敵オブジェクト実装 |
| `Game/Prefabs/PlayerPrefab.h` | プレイヤーPrefabヘッダ |
| `Game/Prefabs/PlayerPrefab.cpp` | プレイヤーPrefab実装 |
| `Game/Prefabs/EnemyPrefab.h` | 敵Prefabヘッダ |
| `Game/Prefabs/EnemyPrefab.cpp` | 敵Prefab実装 |
| `Game/Scenes/GameObjectSlot.h` | GameObjectスロット管理 |

### 6.3 変更ファイル一覧

| ファイル | 変更内容 |
|----------|----------|
| `Engine/Scene/Component.h` | GameObject用のowner設定追加 |
| `Engine/Scene/Component.cpp` | GetGameObject()実装追加 |
| `Engine/Scene/Scene.h` | GameObject管理API追加 |
| `Engine/Scene/Scene.cpp` | GameObject管理ロジック追加 |
| `Engine/Scene/SceneBase.h` | namespace整理 |
| `Engine/Scene/SceneContext.h` | SpawnObject()追加 |
| `Engine/Scene/SceneManager.h` | namespace整理 |

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

#### 派生クラステスト
```cpp
// テスト5: PlayerObject
void Test_PlayerObject_Functionality() {
    Engine::Scene scene;
    scene.Initialize();
    
    auto* player = scene.CreateObject<Game::PlayerObject>("Player1");
    
    ASSERT_EQ(player->GetTag(), "Player");
    ASSERT_EQ(player->GetMoveSpeed(), 5.0f);
    ASSERT_EQ(player->GetHealth(), 100.0f);
    ASSERT_TRUE(player->IsAlive());
    
    player->TakeDamage(50.0f);
    ASSERT_EQ(player->GetHealth(), 50.0f);
    ASSERT_TRUE(player->IsAlive());
    
    player->TakeDamage(60.0f);
    ASSERT_EQ(player->GetHealth(), 0.0f);
    ASSERT_FALSE(player->IsAlive());
}

// テスト6: EnemyObject
void Test_EnemyObject_StateTransitions() {
    Engine::Scene scene;
    scene.Initialize();
    
    auto* player = scene.CreateObject<Game::PlayerObject>("Player");
    auto* enemy = scene.CreateObject<Game::EnemyObject>("Enemy");
    
    enemy->SetTarget(player);
    enemy->SetDetectionRange(10.0f);
    
    // 初期状態はIdle
    ASSERT_EQ(enemy->GetState(), Game::EnemyObject::State::Idle);
    
    // プレイヤーを検知範囲内に配置
    player->SetPosition(Engine::Vector3(5.0f, 0, 0));
    enemy->SetPosition(Engine::Vector3(0, 0, 0));
    
    // Update後に状態遷移
    enemy->Update(0.016f);
    ASSERT_EQ(enemy->GetState(), Game::EnemyObject::State::Chase);
}
```

#### Scene統合テスト
```cpp
// テスト7: Sceneでの統合管理
void Test_Scene_MixedEntityAndGameObject() {
    Engine::Scene scene;
    scene.Initialize();
    
    // Entity生成（既存方式）
    auto* entity = scene.CreateEntity("TestEntity");
    
    // GameObject生成（新方式）
    auto* gameObject = scene.CreateObject<Engine::GameObject>("TestGameObject");
    
    ASSERT_EQ(scene.GetEntityCount(), 1);
    ASSERT_EQ(scene.GetObjectCount(), 1);
    
    // 両方がUpdate/Drawで処理される
    scene.Update(0.016f);
    scene.Draw();
    
    // 破棄
    scene.DestroyEntity(entity);
    scene.DestroyObject(gameObject);
    
    ASSERT_EQ(scene.GetEntityCount(), 0);
    ASSERT_EQ(scene.GetObjectCount(), 0);
}

// テスト8: 検索機能
void Test_Scene_ObjectQueries() {
    Engine::Scene scene;
    scene.Initialize();
    
    auto* player = scene.CreateObject<Game::PlayerObject>("MainPlayer");
    auto* enemy1 = scene.CreateObject<Game::EnemyObject>("Enemy1");
    auto* enemy2 = scene.CreateObject<Game::EnemyObject>("Enemy2");
    
    // 名前で検索
    auto* found = scene.FindObjectByName("MainPlayer");
    ASSERT_EQ(found, player);
    
    // タグで検索
    auto enemies = scene.FindObjectsWithTag("Enemy");
    ASSERT_EQ(enemies.size(), 2);
}
```

### 7.2 テスト実行方法

1. **コンパイル確認**: すべてのファイルがコンパイルできることを確認
2. **基本動作確認**: サンプルシーンでGameObjectが正しく生成・更新・描画されることを確認
3. **既存機能の動作確認**: Entity方式の既存コードが引き続き動作することを確認
4. **メモリリーク確認**: オブジェクトの生成・破棄でメモリリークがないことを確認

### 7.3 影響範囲

| 機能 | 影響 | 確認事項 |
|------|------|----------|
| 既存Entity/Component | 低 | 後方互換性あり、動作に変更なし |
| Scene管理 | 中 | GameObject管理APIが追加、既存APIは維持 |
| Prefab生成 | 中 | SpawnObject()追加、既存Spawn()は維持 |
| シーン遷移 | 低 | SceneManagerの動作に変更なし |

---

## 付録

### A. マイグレーションガイド

既存のコードをGameObjectベースに移行する場合：

1. **Entityベースのオブジェクト**: そのまま使用可能（互換性維持）
2. **新規オブジェクト**: GameObjectを継承して実装
3. **既存ScriptComponent**: GameObjectの派生クラス内に統合を検討

### B. 将来の拡張予定

- [ ] 親子階層（GameObject間の親子関係）
- [ ] プーリングシステム（オブジェクトの再利用）
- [ ] シリアライズ/デシリアライズ（セーブ/ロード対応）
- [ ] イベントシステム統合
