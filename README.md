# DirectX11_Framework

C++20 / DirectX11 でのゲーム制作を効率化するために構築されたフレームワークです。  
このリポジトリは、モジュールごとの役割分担や命名規則の統一、コードの再利用性を最優先に設計されています。

## 主な特徴と目的
- **用途ごとの責務分割**  
  各機能（Window、GraphicsDevice、RenderSystem、Shader、Texture、Materialなど）を明確に責務ごとで分割。自由度と保守性を両立させています。
- **直感的な命名規則**  
  全クラスで同一のタイミングで使用できる同一の関数命名を採用し、利用者が迷わない設計。
- **C++20の活用**  
  モダンC++20の機能（`concepts`や`Range-based for`など）を用いて、型安全性・可読性を向上。
- **DirectX11に最適化した設計**  
  メモリ管理の安全性を担保するため、DirectX11におけるCOMオブジェクトの管理は`ComPtr`を統一的に使用。

---

## アーキテクチャ概要（Unity風：Entity + Componentモデル）

- **Application**  
  メインループ、全体初期化、および終了処理を司り、エンジンのエントリポイントを管理します。
- **Scene**  
  Sceneはゲーム内の状態やオブジェクトを管理するクラスで、Prefabやエンティティの生成、イベントの管理を行います。  
  各ゲーム固有のロジックは`SceneBase`の派生クラス内に記述。
- **Entity (GameObject)**  
  オブジェクトはすべて`Entity`クラスで表現され、必要な機能は任意の`Component`によって拡張可能です。
- **Component**  
  各エンティティに機能を追加する「部品」。例: `Transform`（位置やスケール管理）、`MeshRenderer`、`Camera`、`ScriptComponent`（ロジック記述用）など。
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

3. **エンティティ操作**:
   - `AddXxx() / RemoveXxx() / ClearXxx()`: エンティティやコンポーネントなどの追加/削除。

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
