# DirectX11_Framework

C++ / DirectX11 でゲーム制作を行うためのフレームワークです。  
このリポジトリでは「用途ごとに cpp を分けやすい構造」「語彙（関数名）を固定して意味を統一」「外部から状態を壊せない設計」を最優先にします。

---

## 目標

- **用途ごとに分離**：Window / GraphicsDevice / Render / Shader / Texture / Material / Scene / Entity / Component … を責務で分割
- **語彙（関数名）を固定**：全クラスで同じ意味・同じタイミングで呼ばれるように統一
- **カプセル化の徹底**：変数は private が基本。状態変更は用途別の関数経由のみ
- **DirectX11 の COM は ComPtr 統一**：手動 Release() をしない。借用は raw pointer 返しOK

---

## アーキテクチャ概要（Unity 風：Entity + Component）

- **Application** がメインループと全体初期化を司る
- **World(Scene)** が Entity 群を管理し、Update/Draw の呼び出しを統括
- **Entity(GameObject相当)** は Component の器
- **Component** は機能単位（Transform / MeshRenderer / Camera / ScriptComponent など）
- **ScriptComponent** はゲームロジックを書くための Component（MonoBehaviour の代替）

> 重たい基盤（Window / GraphicsDevice / RenderSystem / AssetManager）は Engine 側が保持し、  
> ScriptComponent は「使う側」に徹します（D3D11 の内部実装を直接触らない設計）。

---

## ディレクトリ構成（例）

DirectX11_Framework/
Engine/
Core/
Application.h / Application.cpp
Time.h / Time.cpp
Logger.h / Logger.cpp
Platform/
Window.h / Window.cpp
Input.h / Input.cpp
Graphics/
GraphicsDevice.h / GraphicsDevice.cpp
RenderSystem.h / RenderSystem.cpp
Shader.h / Shader.cpp
Texture.h / Texture.cpp
Mesh.h / Mesh.cpp
Material.h / Material.cpp
Resources/
AssetManager.h / AssetManager.cpp
Scene/
World.h / World.cpp
Entity.h / Entity.cpp
Component.h / Component.cpp
Components/
Transform.h / Transform.cpp
MeshRenderer.h / MeshRenderer.cpp
Camera.h / Camera.cpp
ScriptComponent.h / ScriptComponent.cpp
Game/
Scripts/
PlayerController.h / PlayerController.cpp


---

## クラスの責務（要点）

### Core
- **Application**
  - 初期化 / ループ / 終了の順序を管理（司令塔）
  - Window / GraphicsDevice / RenderSystem / World / Time を所有

- **Time**
  - deltaTime、FPS、フレーム計測など

### Platform
- **Window**
  - Win32 ウィンドウ生成、メッセージポンプ、サイズ変更通知など

### Graphics
- **GraphicsDevice**
  - D3D11 device / context / swapchain / RTV / DSV の管理
  - Present, Resize, Clear などの低レベル API

- **RenderSystem**
  - 1フレームの描画手順（Clear → RenderQueue処理 → Present）
  - 描画順序、状態（State）切替、必要なら将来ポストエフェクトもここ

- **Shader / Texture / Mesh / Material**
  - GPUリソースの薄いラッパ（中身は ComPtr を持つ）

### Resources
- **AssetManager**
  - 読み込みとキャッシュ（同一パスは同一リソースを返す）
  - Load と管理の入口を統一する

### Scene / Components
- **World**
  - Entity 群の更新と描画を統括（生成・破棄の安全管理もここ）

- **Entity**
  - Component の器
  - Add / Remove / Get を提供
  - Transform は標準で必須扱いにする設計が推奨

- **Component（基底）**
  - owner（所属 Entity）と enabled を持つ
  - OnAwake / OnStart / Update / LateUpdate / Draw / OnDestroy を提供

- **Transform**
  - 位置/回転/拡縮の管理（行列生成）
  - 値は private で保持し、用途別関数（Translate / LookAt 等）経由で操作

- **MeshRenderer**
  - Mesh + Material を保持し、RenderSystem に「描画要求」を出す

- **Camera**
  - View/Projection の管理

- **ScriptComponent**
  - ユーザーが継承してゲームロジックを書く場所（Updateなど）

---

## 固定語彙（関数名）ルール

### ライフサイクル（推奨）
- **Initialize()**
  - 依存関係の準備・初期化（※生成とは別）
- **Finalize()**
  - 後始末（安全に複数回呼べる設計が望ましい）
- **Reset()**
  - 状態の初期化へ戻す（ゲーム内リトライ等を想定）
- **Update(float deltaTime)**
  - 毎フレーム更新（ロジック）
- **Draw()**
  - 毎フレーム描画（描画要求の送出）

### リソース系（責務の意味を固定）
- **Load() / Unload()**
  - ファイルや外部資産の読み込み/解放
- **Register() / Unregister()**
  - システムへ登録/解除（RenderSystem へ登録、イベント購読、管理対象追加など）
- **Enable() / Disable() / IsEnabled()**
  - 有効/無効切替（Update/Drawの対象制御）

### コンテナ/所有物操作
- **AddXxx() / RemoveXxx() / ClearXxx()**
  - 要素の追加/削除/全消去

### 生成・破棄（意味を固定）
- **Create() / Destroy()**
  - “生成・破棄” だけを意味する  
  - **Create に Load/Register を含めない（混在禁止）**

---

## 呼び出し順序（Application 例）

1. `Application::Initialize()`
2. `World::Initialize()`, `RenderSystem::Initialize()`, etc...
3. 必要に応じて `Load()` → `Register()`
4. ループ
   - `Window::PumpMessages()`
   - `Time::Tick()`
   - `World::Update(dt)`
   - `World::LateUpdate(dt)`（必要なら）
   - `RenderSystem::Draw(World)`（RenderQueue収集→描画）
5. `Unregister()` → `Unload()`（必要に応じて）
6. `Application::Finalize()`

---

## 命名規則（厳守）

### 型
- クラス / 構造体 / enum：`UpperCamelCase`
  - 例：`GraphicsDevice`, `RenderSystem`, `ComponentType`

### 変数
- ローカル / 引数：`lowerCamelCase`
  - 例：`deltaTime`, `assetPath`
- メンバ変数：`m_lowerCamelCase`
  - 例：`m_device`, `m_isEnabled`
- static（可変の共有）：`s_lowerCamelCase`
  - 例：`s_instanceCount`
- 定数（不変）：`kUpperCamelCase`
  - 例：`kMaxSpeed`, `kDefaultFov`
- bool 型は **Is を付ける**
  - 例：`IsVisible()`, `m_isActive`, `m_isEnabled`

### ファイル名
- ファイル名：`UpperCamelCase`
  - 例：`GraphicsDevice.cpp`, `ScriptComponent.h`

---

## const ルール

### 関数 const
- **状態を変えない関数は必ず const**
  - 例：`int GetHp() const;`

### 引数 const
- 小さい型（int/float/bool/enum/ポインタなど）：**値渡し**
- 大きい型（std::string / std::vector / 行列 / 大きい構造体など）：**const&**

### Get の返し方
- 小さい型（int/float/bool/enum/ポインタ）：**値返し**
  - 例：`int GetHp() const;`
- 大きい型（Vector/Matrix/string/コンテナ）：**const参照返し**
  - 例：`const Matrix4& GetWorldMatrix() const;`
- **非const参照 Get は原則禁止**

---

## DirectX11 / COM ルール（統一）

- COM は **ComPtr** を基本とする
- **手動 Release() はしない**
- Getter で外へ出す場合は “借用” として raw pointer を返してよい
  - 例：`ID3D11Device* GetDevice() const { return m_device.Get(); }`
- 外部で保持が必要なら、保持側も **ComPtr** で受ける（プロジェクト内統一）

---

## カプセル化（状態を壊せない設計）

- 変数は基本 **private**
- 状態変更は用途別の関数でのみ行う（整合性を守る）
  - 悪い例：`SetPosition(Vector3 p)` を乱用してどこでも瞬間移動
  - 良い例：`TeleportTo()` / `MoveBy()` / `ClampToArea()` など **意図が分かる API**
- “不変条件” をクラス内で守る（例：Scale が 0 にならない、角度正規化、範囲 clamp 等）

---

## ScriptComponent（ゲームロジック）の方針

- ScriptComponent は **ゲームロジックを書く場所**
- Window / GraphicsDevice / Shader の管理は Engine 側へ寄せる
- Script からは Entity/Component 経由で必要なものへアクセスする
  - 例：`GetOwner()->GetComponent<Transform>()`

---

## コーディングメモ（推奨）

- 可能な限り `override` を付ける
- 破棄系は安全に多重呼び出し可能にする（Finalize/Destroy など）
- `Create()` と `Initialize()` の意味を混ぜない（ルール固定）

---

## ライセンス

- 現状無し
