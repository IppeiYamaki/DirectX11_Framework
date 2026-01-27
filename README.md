# DirectX11_Framework

C++20とDirectX11を使用してゲーム制作を行うためのフレームワークです。  
このリポジトリでは「用途ごとの分割」と「汎用性のある設計」を重視し、ゲーム制作を効率化することを目的としています。

本フレームワークは、Unity風のアーキテクチャ（Entity + Componentモデル）を参考に設計され、柔軟性やカスタマイズ性を確保しつつ直感的な実装を可能にすることを目指しています。

---

## 目標

1. **用途ごとの分離**  
   各機能（Window、GraphicsDevice、RenderSystem、Shader、Texture、Material、Scene、Entity、Componentなど）を役割ごとに明確に管理する構成を採用します。

2. **語彙（関数名）の固定化**  
   全クラスで同じ意味の関数を同じタイミングで呼び出せるよう統一的な命名規則を採用。

3. **カプセル化の徹底**  
   保守性と安全性を高めるため、変数はprivateを基本とし、状態の変更は用途別の関数を通じて行います。

4. **DirectX11環境に最適化**  
   DirectX11のCOMは`ComPtr`で統一し、手動`Release()`は行わず、メモリ管理の安全性を確保しています。

5. **最新のC++20を活用**  
   C++20の新機能（concepts、coroutines、range-based forなど）を活用し、モダンなC++スタイルを実現。

---

## アーキテクチャ概要

- **Application**: ゲームのメインループを管理し、全体の初期化・終了処理を司る。
- **Scene**: UnityのWorldに相当するクラスであり、通常のエンティティやゲーム内の管理を行う。
- **Entity (GameObject)**: ゲーム内オブジェクトを表現するクラスであり、Componentを組み合わせて機能を定義。
- **Component**: 機能単位を表すクラス。例としてTransform、MeshRenderer、Camera、ScriptComponentなどがある。

**特記事項**:
- 「重たい処理」（Window、GraphicsDevice、RenderSystem、AssetManagerなど）を含む基盤の実装は`Engine`側が担当。
- ゲームロジックを記述する`ScriptComponent`では、ゲーム開発者が直接エンジン内部のリソース（D3D11など）を操作しなくてもよいように設計されています。

---

## 主要機能と設計指針

### **Graphics (DirectX11基盤)**

- **GraphicsDevice**: D3D11の`device`や`context`、`swapchain`を管理。
- **RenderSystem**: 描画手順や状態切り替えを統括し、将来的なポストエフェクトの拡張も想定済み。
- **Material / Shader / Texture / Mesh**: GPUリソースの管理を担当し、高い柔軟性を実現。

### **Scene/Entity/Component**

- **Scene（SceneBase）**:  
  ゲームの状態を管理するベースクラスです。それぞれのSceneはPrefabやEntityを生成し、描画や更新処理を一元管理します。ゲーム固有のSceneごとに継承して作成可能です。

- **Entity**:  
  ゲームオブジェクトを表現するクラスで、`Component`を組み合わせて機能を拡張可能です。

- **Transform / Component**:  
  すべてのエンティティが保持するコンポーネントで、位置、回転、拡縮の管理を行います。  
  各種機能単位（MeshRendererやCameraなど）はComponentとして実装されています。

- **PrefabとPrefabManager**:  
  UnityのPrefab用の仕組みを参考に設計されており、Prefabごとのスナップショットを元に動的なエンティティ生成を行えます。

### **Camera（カメラ管理システム）**

カメラの生成・利用方法を統一するため、次の仕組みを用意しました：
- エンジン内で`CameraSystem`を設計し、カメラの生成・削除・設定を統一。
- メインカメラの切り替え機能を提供し、ゲーム内でメインの視点を動的に切り替えることをサポート。

```cpp
// カメラの使用例
Engine::Camera* mainCamera = ctx.m_cameraSystem->AddCamera({position, rotation, fov, nearZ, farZ});
ctx.m_cameraSystem->SetMainCamera(mainCamera); // メインカメラを設定
```

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
    CameraSystem.h / CameraSystem.cpp
  UI/
    Canvas.h / Canvas.cpp
    UIElement.h / UIElement.cpp
Game/
  Scenes/
    TitleScene.h / TitleScene.cpp
    GameScene.h / GameScene.cpp
  Prefabs/
    MainCameraPrefab.h / SamplePrefab.h
  Scripts/
    PlayerLogic.h / PlayerLogic.cpp
```

---

## 活用技術

- 言語: **C++20**
- グラフィックスAPI: **DirectX11**
- アーキテクチャ: Unity風 **Entity + Componentモデル**
- メモリ管理: **ComPtrベースで安全性を確保**

---

## ライセンス

- 現在、本リポジトリに対して適用されるライセンスは設定されていません。
