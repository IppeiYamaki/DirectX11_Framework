# DirectX11_Framework

C++20 / DirectX11 でのゲーム制作を効率化するために構築されたフレームワークです。  
このリポジトリは、モジュールごとの役割分担や命名規則の統一、コードの再利用性を最優先に設計されています。

本フレームワークは、Unity風のアーキテクチャ（GameObject + Componentモデル）を参考に設計され、柔軟性やカスタマイズ性を確保しつつ直感的な実装を可能にすることを目指しています。

---

## 主な特徴と目的
- **用途ごとの責務分割**  
  各機能（Window、GraphicsDevice、RenderSystem、Shader、Texture、Materialなど）を明確に責務ごとで分割。自由度と保守性を両立させています。
- **直感的な命名規則**  
   全クラスで同じ意味の関数を同じタイミングで呼び出せるよう統一的な命名規則を採用。
- **カプセル化の徹底**  
   保守性と安全性を高めるため、変数はprivateを基本とし、状態の変更は用途別の関数を通じて行います。
- **C++20の活用**  
  モダンC++20の機能（`concepts`や`Range-based for`など）を用いて、型安全性・可読性を向上。
- **DirectX11に最適化した設計**  
  DirectX11のCOMは`ComPtr`で統一し、手動`Release()`は行わず、メモリ管理の安全性を確保しています。

---

## アーキテクチャ概要（Unity風：GameObject + Componentモデル）

- **Application**  
  メインループ、全体初期化、および終了処理を司り、エンジンのエントリポイントを管理します。
- **Scene**  
  Sceneはゲーム内の状態やオブジェクトを管理するクラスで、PrefabやGameObjectの生成、イベントの管理を行います。  
  各ゲーム固有のロジックは`SceneBase`の派生クラス内に記述。
- **GameObject**  
  オブジェクトはすべて`GameObject`クラスで表現され、必要な機能は任意の`Component`によって拡張可能です。
  派生クラス（PlayerObject, EnemyObjectなど）を作成してオブジェクト固有の挙動を実装することも可能です。
- **Component**  
  各GameObjectに機能を追加する「部品」。例: `Transform`（位置やスケール管理）、`MeshRenderer`、`Camera`、`ScriptComponent`（ロジック記述用）など。
- **Prefab**  
  プレハブシステムは事前にテンプレート化されたゲームオブジェクトを管理し、動的にゲーム内でインスタンス化する仕組み。

---

## コード編集ルール

### 命名規則
開発者間で一貫性を保つために、以下の命名規則を厳守してください：
- **クラス/構造体/enum**: `UpperCamelCase` （例: `GraphicsDevice`, `SceneBase`）
- **ローカル変数/引数**: `lowerCamelCase` （例: `deltaTime`, `assetPath`）
- **メンバ変数**: `m_lowerCamelCase` （例: `m_device`, `m_isEnabled`）
- **static変数**: `s_lowerCamelCase` （例: `s_instanceCount`）
- **定数**: `kUpperCamelCase` （例: `kMaxSpeed`, `kDefaultFov`）
- **bool型変数**には`Is`や`Has`を頭に付ける （例: `IsVisible`, `m_isActive`, `HasTag()`）。

---

## 主要機能と設計指針

### **Graphics (DirectX11基盤)**

- **GraphicsDevice**: D3D11の`device`や`context`、`swapchain`を管理。
- **RenderSystem**: 描画手順や状態切り替えを統括し、将来的なポストエフェクトの拡張も想定済み。
- **Material / Shader / Texture / Mesh**: GPUリソースの管理を担当し、高い柔軟性を実現。

### **Scene/GameObject/Component**

- **Scene（SceneBase）**:  
  ゲームの状態を管理するベースクラスです。それぞれのSceneはPrefabやGameObjectを生成し、描画や更新処理を一元管理します。ゲーム固有のSceneごとに継承して作成可能です。

- **GameObject**:  
  ゲームオブジェクトを表現する基底クラスで、`Component`を組み合わせて機能を拡張可能です。
  派生クラスを作成してオブジェクト固有の挙動を実装することも可能です。

- **Transform / Component**:  
  すべてのGameObjectが保持するコンポーネントで、位置、回転、拡縮の管理を行います。  
  各種機能単位（MeshRendererやCameraなど）はComponentとして実装されています。

- **PrefabとPrefabManager**:  
  UnityのPrefab用の仕組みを参考に設計されており、Prefabごとのスナップショットを元に動的なGameObject生成を行えます。

### **Camera（カメラ管理システム）**

カメラの生成・利用方法を統一するため、次の仕組みを用意しました：
- エンジン内で`CameraSystem`を設計し、カメラの生成・削除・設定を統一。
- メインカメラの切り替え機能を提供し、ゲーム内でメインの視点を動的に切り替えることをサポート。

```cpp
// カメラの使用例
Engine::Camera* mainCamera = ctx.m_cameraSystem->AddCamera({position, rotation, fov, nearZ, farZ});
ctx.m_cameraSystem->SetMainCamera(mainCamera); // メインカメラを設定
```

### **Light（光源管理システム）**

複数種類の光源を統一的に管理するための`LightSystem`を設計しました：
- 方向性ライト（DirectionalLight）：太陽光のような無限遠からの光
- 点光源（PointLight）：全方向に放射される光
- スポットライト（SpotLight）：円錐状に放射される光

```cpp
// 光源の使用例
void GameScene::BuildScene(Engine::SceneContext& ctx) {
    // Directional Light を追加
    auto* dirLight = ctx.m_lightSystem->AddDirectionalLight({1.0f, -1.0f, 1.0f});
    dirLight->SetColor({1.0f, 0.9f, 0.8f});
    dirLight->SetIntensity(1.5f);

    // Point Light を追加
    auto* pointLight = ctx.m_lightSystem->AddPointLight({5.0f, 10.0f, 5.0f});
    pointLight->SetColor({0.8f, 0.8f, 1.0f});
    pointLight->SetIntensity(2.0f);

    // Spot Light を追加
    auto* spotLight = ctx.m_lightSystem->AddSpotLight({0.0f, 5.0f, -5.0f}, {0.0f, -1.0f, 0.0f}, 30.0f, 40.0f);
    spotLight->SetColor({1.0f, 1.0f, 0.9f});
    spotLight->SetIntensity(3.5f);
}
```

詳細については [LIGHTSYSTEM_DESIGN.md](Docs/LIGHTSYSTEM_DESIGN.md) を参照してください。

### **Canvas UIシステム**

UnityのCanvas機能を参考に、UI要素を手軽に管理・配置できる`Canvas`システムを設計しました：
- UIがゲーム内オブジェクトよりも手前に表示されるよう、描画順序を管理。
- `Canvas`を利用することで、ボタンやテキスト、イメージなどを簡単に扱える。
```cpp
// Canvas の利用例
auto canvas = std::make_unique<Engine::Canvas>();

// ボタンやテキストなどを Canvas に追加
auto button = std::make_unique<MyButton>();
canvas->AddElement(std::move(button));

// 描画
canvas->Render();
```

---

## ディレクトリ構成

```
DirectX11_Framework/
Engine/
  Core/
  Platform/
  Graphics/
  Resources/
  Scene/
    SceneBase.h / SceneBase.cpp
    GameObject.h / GameObject.cpp
    CameraSystem.h / CameraSystem.cpp
    LightSystem.h / LightSystem.cpp
    Light.h / Light.cpp
  UI/
    Canvas.h / Canvas.cpp
    UIElement.h / UIElement.cpp
Game/
  GameObjects/
    PlayerObject.h / PlayerObject.cpp
    EnemyObject.h / EnemyObject.cpp
  Scenes/
    TitleScene.h / TitleScene.cpp
    GameScene.h / GameScene.cpp
  Prefabs/
    MainCameraPrefab.h / SamplePrefab.h
  Scripts/
    PlayerLogic.h / PlayerLogic.cpp
Docs/
  LIGHTSYSTEM_DESIGN.md
  CAMERASYSTEM_CANVAS_DESIGN.md
```

---

## 活用技術

- 言語: **C++20**
- グラフィックスAPI: **DirectX11**
- アーキテクチャ: Unity風 **GameObject + Componentモデル**
- メモリ管理: **ComPtrベースで安全性を確保**

---


### constのルール
- 状態を変えない関数は**必ず`const`を付与**
  - 例: `int GetCount() const;`
- 引数はサイズにより以下の規則を適用:
  1. **小さい型の値（int, float, bool, enumなど）**: 値渡し
  2. **大きい型（std::string, std::vectorなど）**: `const&`渡し
- 非const参照でのGet関数は**禁止**。

---

### 固定語彙（関数名統一ルール）
1. **ライフサイクル関数**:
   - `Initialize()`: 初期化処理（依存関係の準備など）。
   - `Finalize()`: 終了処理（安全に複数回呼べる設計が推奨）。
   - `Update(float deltaTime)`: 毎フレームごとのロジック更新。
   - `Draw()`: 毎フレームの描画処理。

2. **リソース操作**:
   - `Load()`/`Unload()`: 資源のロードまたはアンロード操作。
   - `Register()`/`Unregister()`: RenderQueueやイベント管理に追加/削除。

3. **オブジェクト操作**:
   - `AddXxx() / RemoveXxx() / ClearXxx()`: GameObjectやコンポーネントなどの追加/削除。

4. **生成/破棄**:
   - `Create()`/`Destroy()`は、生成・破棄のみを意味し、混在操作を禁止。

---

### DirectX11およびComPtrポリシー
- COMオブジェクト（Direct3Dのデバイス、コンテキストなど）には`ComPtr`を使用し、明示的な`Release()`を避けます。
- raw pointerを外部に出す場合は、借用（`ComPtr::Get()`利用）のみ可能。

---

## ライセンス
現在、本リポジトリにはライセンスが適用されておらず、使用上の制限なく利用できます。

---
