# CameraSystem & Canvas UI システム設計書

このドキュメントは、DirectX11_Frameworkに追加されたCameraSystemとCanvas UIシステムについて説明します。

---

## 目次

1. [概要](#1-概要)
2. [CameraSystem](#2-camerasystem)
3. [Canvas UIシステム](#3-canvas-uiシステム)
4. [使用例](#4-使用例)
5. [ファイル構成](#5-ファイル構成)

---

## 1. 概要

### 1.1 背景

従来のフレームワークでは、カメラはPrefabとして作成・管理されていましたが、以下の課題がありました：
- カメラの生成・管理がゲーム側に分散
- メインカメラの切り替えが煩雑
- UIをゲームオブジェクトの手前に表示する仕組みがない

### 1.2 解決策

1. **CameraSystem**: カメラの生成・管理をエンジン側に集約
2. **Canvas**: UI要素を画面手前に描画するシステムを追加

---

## 2. CameraSystem

### 2.1 概要

`CameraSystem`は、カメラの生成・管理を統括するエンジン側のシステムです。

### 2.2 主な機能

- カメラの生成（`AddCamera`）
- カメラの削除（`RemoveCamera`）
- メインカメラの設定・取得（`SetMainCamera` / `GetMainCamera`）
- 複数カメラの管理

### 2.3 クラス設計

```cpp
// カメラ初期化パラメータ
struct CameraInitParams {
    Vector3 m_position{0, 0, -10};  // 初期位置
    float m_yawDeg = 0.0f;          // Yaw角度（度）
    float m_pitchDeg = 0.0f;        // Pitch角度（度）
    float m_rollDeg = 0.0f;         // Roll角度（度）
    float m_fovYRad = XM_PIDIV4;    // 垂直FOV（ラジアン）
    float m_aspect = 16.0f / 9.0f;  // アスペクト比
    float m_nearZ = 0.1f;           // 近クリップ面
    float m_farZ = 1000.0f;         // 遠クリップ面
    bool m_isMain = false;          // メインカメラ設定
};

class CameraSystem {
public:
    bool Initialize(Scene* scene, RenderSystem* renderSystem);
    void Finalize();

    Camera* AddCamera(const CameraInitParams& params);
    void RemoveCamera(Camera* camera);

    void SetMainCamera(Camera* camera);
    Camera* GetMainCamera() const;

    std::size_t GetCameraCount() const;
    void Update(float deltaTime);
};
```

### 2.4 使用方法

```cpp
void SampleScene::BuildScene(SceneContext& ctx) {
    // CameraSystemを使用してカメラを作成
    CameraInitParams params{};
    params.m_position = Vector3(0, 3, -8);
    params.m_pitchDeg = -5.0f;
    params.m_isMain = true;

    auto* mainCamera = ctx.m_cameraSystem->AddCamera(params);
    ctx.m_cameraSystem->SetMainCamera(mainCamera);
}
```

---

## 3. Canvas UIシステム

### 3.1 概要

`Canvas`は、UI要素を画面手前に描画するUnity風のシステムです。

### 3.2 主な機能

- UI要素の追加・削除
- 描画順序の管理（ソート順）
- スクリーンスペースレンダリング

### 3.3 クラス設計

#### UIElement（基底クラス）

```cpp
class UIElement {
public:
    virtual void Initialize();
    virtual void Update(float deltaTime);
    virtual void Render(RenderSystem* renderSystem);
    virtual void OnDestroy();

    void SetPosition(const Vector2& position);
    const Vector2& GetPosition() const;

    void SetSize(const Vector2& size);
    const Vector2& GetSize() const;

    void Enable();
    void Disable();
    bool IsEnabled() const;

    void SetVisible(bool visible);
    bool IsVisible() const;

    void SetSortOrder(int order);
    int GetSortOrder() const;
};
```

#### Canvas

```cpp
class Canvas {
public:
    bool Initialize(float screenWidth, float screenHeight);
    void Finalize();

    UIElement* AddElement(std::unique_ptr<UIElement> element);
    
    template <class T, class... Args>
    T* CreateElement(Args&&... args);

    void RemoveElement(UIElement* element);
    void ClearElements();

    void Update(float deltaTime);
    void Render(RenderSystem* renderSystem);

    void SetScreenSize(float width, float height);
    
    void Enable();
    void Disable();
    bool IsEnabled() const;
};
```

### 3.4 使用方法

```cpp
void MyScene::Initialize(SceneContext& ctx) {
    if (ctx.m_canvas) {
        // カスタムUI要素を追加
        auto button = std::make_unique<MyButton>();
        button->SetPosition(Vector2(100, 50));
        button->SetSize(Vector2(200, 50));
        ctx.m_canvas->AddElement(std::move(button));
    }
}
```

### 3.5 描画順序

Canvas内のUI要素は`SortOrder`によってソートされます：
- 小さい値ほど先に描画（奥に表示）
- 大きい値ほど後に描画（手前に表示）

---

## 4. 使用例

### 4.1 SceneでのCameraSystem使用

```cpp
void GameplayScene::BuildScene(SceneContext& ctx) {
    // メインカメラを作成
    CameraInitParams mainParams{};
    mainParams.m_position = Vector3(0, 5, -15);
    mainParams.m_pitchDeg = -10.0f;
    mainParams.m_isMain = true;
    auto* mainCamera = ctx.m_cameraSystem->AddCamera(mainParams);

    // サブカメラを作成（ミニマップ用など）
    CameraInitParams subParams{};
    subParams.m_position = Vector3(0, 50, 0);
    subParams.m_pitchDeg = -90.0f;  // 真下を向く
    auto* topDownCamera = ctx.m_cameraSystem->AddCamera(subParams);

    // メインカメラを切り替え
    ctx.m_cameraSystem->SetMainCamera(mainCamera);
}
```

### 4.2 Canvas UIの使用

```cpp
// カスタムUI要素の作成
class HealthBar : public UIElement {
public:
    void Initialize() override {
        // 初期化処理
    }

    void Update(float deltaTime) override {
        // 毎フレーム更新
    }

    void Render(RenderSystem* rs) override {
        // 描画処理（スクリーンスペース）
    }

private:
    float m_currentHealth = 100.0f;
    float m_maxHealth = 100.0f;
};

// Sceneでの使用
void GameScene::Initialize(SceneContext& ctx) {
    auto healthBar = std::make_unique<HealthBar>();
    healthBar->SetPosition(Vector2(10, 10));
    healthBar->SetSize(Vector2(200, 20));
    healthBar->SetSortOrder(100);  // 手前に表示
    ctx.m_canvas->AddElement(std::move(healthBar));
}
```

---

## 5. ファイル構成

```
Source/
├── Engine/
│   ├── Scene/
│   │   ├── CameraSystem.h       ★ カメラ管理システムヘッダ
│   │   ├── CameraSystem.cpp     ★ カメラ管理システム実装
│   │   ├── World.h              ★ World統括クラスヘッダ
│   │   ├── World.cpp            ★ World統括クラス実装
│   │   ├── SceneContext.h       （更新）CameraSystem/Canvas追加
│   │   └── ...
│   │
│   └── UI/                      ★ 新規ディレクトリ
│       ├── Canvas.h             ★ Canvasクラスヘッダ
│       ├── Canvas.cpp           ★ Canvasクラス実装
│       ├── UIElement.h          ★ UI要素基底クラスヘッダ
│       └── UIElement.cpp        ★ UI要素基底クラス実装
│
└── Game/
    ├── GameMain.h               （更新）CameraSystem/Canvas管理追加
    ├── GameMain.cpp             （更新）初期化・更新・描画追加
    └── Scenes/
        └── SampleScene.cpp      （更新）CameraSystem使用例
```

---

## 6. 依存関係

```
GameMain
    │
    ├── CameraSystem ─────────────────┐
    │       │                         │
    │       └── Camera (Component)    │
    │           └── Transform         │
    │                                  │
    └── Canvas ───────────────────────┤
            │                         │
            └── UIElement (abstract)  │
                └── (派生クラス)       │
                                      │
SceneContext ─────────────────────────┘
    │
    └── (各Sceneから参照)
```

---

## 7. 今後の拡張予定

- [ ] UI要素の派生クラス追加（Button、Text、Image等）
- [ ] UIイベントシステム（クリック、ホバー等）
- [ ] カメラ切り替えアニメーション
- [ ] 複数カメラ同時描画（スプリットスクリーン）
- [ ] UIアンカーシステム（画面端配置）
