# GameObject アーキテクチャ設計

このドキュメントは、DirectX11_FrameworkのGameObjectシステムの設計を説明します。

---

## 概要

GameObjectシステムは、Unityを参考にした設計で、ゲーム内のすべてのオブジェクトを統一的に管理するための基盤を提供します。

### 設計思想

1. **GameObjectは「器」として設計** - Transform、名前、タグ、コンポーネントリストを保持
2. **具体的な振る舞いは派生クラスで定義** - PlayerObject、EnemyObjectなど
3. **自己完結的なロジック** - 各GameObjectが自身の振る舞いを管理

---

## クラス構成

### 1. Engine::GameObject (基底クラス)

`Source/Engine/Scene/GameObject.h`

```cpp
class GameObject {
public:
    explicit GameObject(const std::string& name = "");
    virtual ~GameObject();

    // ライフサイクル
    virtual void Initialize();          // 初期化処理
    virtual void Update(float deltaTime); // 毎フレーム更新
    virtual void LateUpdate(float deltaTime); // 遅延更新
    virtual void Render();              // 描画処理
    virtual void OnDestroy();           // 破棄時処理

    // Transform操作
    void SetPosition(const Vector3& position);
    void SetRotation(const Vector3& rotation);
    void SetScale(const Vector3& scale);
    Vector3 GetPosition() const;
    Vector3 GetRotation() const;
    Vector3 GetScale() const;

    // 識別
    void SetName(const std::string& name);
    const std::string& GetName() const;
    void SetTag(const std::string& tag);
    bool CompareTag(const std::string& tag) const;

    // 有効/無効
    void SetActive(bool active);
    bool IsActive() const;

    // コンポーネント管理
    template<class T, class... Args>
    T* AddComponent(Args&&... args);
    template<class T>
    T* GetComponent();

protected:
    std::string m_name;
    std::string m_tag;
    bool m_isActive;
    Transform* m_transform;
    std::vector<std::unique_ptr<Component>> m_components;
};
```

### 2. 派生クラス例

#### Game::PlayerObject

`Source/Game/Objects/PlayerObject.h`

```cpp
class PlayerObject : public Engine::GameObject {
public:
    explicit PlayerObject(const std::string& name = "Player");

    void Initialize() override;
    void Update(float deltaTime) override;
    void Render() override;

    // プレイヤー固有機能
    void SetMoveSpeed(float speed);
    float GetMoveSpeed() const;
    void SetHP(int hp);
    int GetHP() const;
    void TakeDamage(int damage);
    bool IsDead() const;

private:
    void HandleInput(float deltaTime);

    float m_moveSpeed = 5.0f;
    int m_hp = 100;
};
```

#### Game::EnemyObject

`Source/Game/Objects/EnemyObject.h`

```cpp
class EnemyObject : public Engine::GameObject {
public:
    explicit EnemyObject(const std::string& name = "Enemy");

    void Initialize() override;
    void Update(float deltaTime) override;
    void Render() override;
    void OnDestroy() override;

    // 敵固有機能
    void SetTarget(GameObject* target);
    void SetBehavior(EnemyBehavior behavior);
    void SetMoveSpeed(float speed);
    void SetDetectionRange(float range);
    void TakeDamage(int damage);

private:
    void UpdateIdle(float deltaTime);
    void UpdatePatrol(float deltaTime);
    void UpdateChase(float deltaTime);
    void UpdateAttack(float deltaTime);

    GameObject* m_target = nullptr;
    EnemyBehavior m_behavior = EnemyBehavior::Idle;
    float m_moveSpeed = 3.0f;
    float m_detectionRange = 10.0f;
};
```

---

## World クラスの拡張

`Source/Engine/Scene/World.h`

Worldクラスは、EntityとGameObjectの両方を管理します：

```cpp
class World {
public:
    // GameObject生成
    template<class T, class... Args>
    T* CreateGameObject(Args&&... args);

    GameObject* AddGameObject(std::unique_ptr<GameObject> object);
    void DestroyGameObject(GameObject* object);
    void DestroyGameObjectDeferred(GameObject* object);

    // 検索
    GameObject* FindGameObjectByName(const std::string& name);
    std::vector<GameObject*> FindGameObjectsWithTag(const std::string& tag);
    template<typename T>
    std::vector<GameObject*> FindGameObjectsWithComponent();

private:
    std::vector<std::unique_ptr<Entity>> m_entities;
    std::vector<std::unique_ptr<GameObject>> m_gameObjects;
};
```

---

## Prefabシステム

### GameObjectPrefab

`Source/Game/Prefabs/GameObjectPrefab.h`

```cpp
struct GameObjectSpawnDesc {
    Vector3 m_position{ 0, 0, 0 };
    Vector3 m_rotation{ 0, 0, 0 };
    Vector3 m_scale{ 1, 1, 1 };
};

template<typename TGameObject, typename TSpawnDesc = GameObjectSpawnDesc>
class GameObjectPrefab {
public:
    using SpawnDesc = TSpawnDesc;
    static TGameObject* Spawn(WorldContext& ctx, const TSpawnDesc& desc);
};
```

### 具体的なPrefab例

```cpp
// PlayerPrefab.h
struct PlayerSpawnDesc : public GameObjectSpawnDesc {
    float m_moveSpeed = 5.0f;
    int m_hp = 100;
};

class PlayerPrefab {
public:
    using SpawnDesc = PlayerSpawnDesc;
    static PlayerObject* Spawn(WorldContext& ctx, const SpawnDesc& desc);
};
```

---

## 使用例

### WorldでのGameObject生成

```cpp
void SampleWorld::BuildWorld(WorldContext& ctx) {
    // 直接生成
    auto* player = ctx.m_world->CreateGameObject<PlayerObject>("Player1");
    player->SetPosition(Vector3(0, 0, 0));
    player->SetMoveSpeed(10.0f);

    // Prefabを使用した生成
    auto* enemy = ctx.SpawnGameObject<EnemyPrefab>(
        Vector3(5, 0, 0),  // 位置
        3.0f,               // 移動速度
        15.0f,              // 検知範囲
        100                 // HP
    );
    enemy->SetTarget(player);
}
```

---

## ディレクトリ構成

```
Source/
├── Engine/
│   └── Scene/
│       ├── Component.h/.cpp       # 基底Component
│       ├── Entity.h/.cpp          # Entityクラス
│       ├── GameObject.h/.cpp      # GameObjectクラス（新規）
│       └── World.h/.cpp           # World管理
└── Game/
    ├── Objects/                   # GameObjectの派生クラス（新規）
    │   ├── PlayerObject.h/.cpp
    │   └── EnemyObject.h/.cpp
    └── Prefabs/
        ├── GameObjectPrefab.h     # Prefab基底（新規）
        ├── PlayerPrefab.h/.cpp    # プレイヤーPrefab（新規）
        └── EnemyPrefab.h/.cpp     # 敵Prefab（新規）
```

---

## EntityとGameObjectの比較

| 特徴 | Entity | GameObject |
|------|--------|------------|
| 用途 | 軽量コンポーネントコンテナ | 自己完結的なゲームオブジェクト |
| 振る舞い定義 | Componentで定義 | 派生クラスで定義 |
| Transform | Componentとして追加 | 組み込み |
| ライフサイクル | Component経由 | 直接メソッド |
| 推奨用途 | シンプルなオブジェクト | 複雑な振る舞いを持つオブジェクト |

両方のシステムは共存可能で、プロジェクトの要件に応じて選択できます。

---

## 責務分担

### Scene (World)
- GameObject/Entityの生成・管理
- ライフサイクル制御（Update/Draw呼び出し）
- オブジェクト検索・クエリ

### GameObject
- 自身のTransform管理
- 振る舞いの定義（派生クラスで）
- Componentの管理

### Prefab
- オブジェクト生成のテンプレート
- 初期状態の定義
- ファクトリパターンの実装
