# コーディング規約

## 概要

このドキュメントでは、DirectX11_Frameworkにおけるコーディング規約の詳細と、既存コードとの整合性確認結果、改善提案を示します。

---

## 命名規則（README.mdより）

### 型名

| 種別 | 規則 | 例 |
|------|------|-----|
| クラス | UpperCamelCase | `GraphicsDevice`, `RenderSystem` |
| 構造体 | UpperCamelCase | `RenderItem`, `VertexInputElement` |
| enum | UpperCamelCase | `RenderLayer`, `ComponentType` |
| enum class | UpperCamelCase | `RenderStateFlag` |

### 変数名

| 種別 | 規則 | 例 |
|------|------|-----|
| ローカル変数 | lowerCamelCase | `deltaTime`, `assetPath` |
| 引数 | lowerCamelCase | `context`, `position` |
| メンバ変数 | m_lowerCamelCase | `m_device`, `m_isEnabled` |
| static変数 | s_lowerCamelCase | `s_instanceCount` |
| 定数 | kUpperCamelCase | `kMaxSpeed`, `kDefaultFov` |
| bool型 | Is を付ける | `IsVisible()`, `m_isActive` |

### 関数名

| 種別 | 規則 | 例 |
|------|------|-----|
| 関数 | UpperCamelCase | `Initialize`, `GetPosition` |
| ゲッター | Get + 名詞 | `GetOwner()`, `GetDevice()` |
| セッター | Set + 名詞 | `SetPosition()`, `SetEnabled()` |
| bool取得 | Is + 形容詞 | `IsEnabled()`, `IsInitialized()` |

### ファイル名

| 種別 | 規則 | 例 |
|------|------|-----|
| ヘッダー | UpperCamelCase.h | `GraphicsDevice.h` |
| ソース | UpperCamelCase.cpp | `GraphicsDevice.cpp` |
| シェーダー | UpperCamelCase.hlsl | `DefaultVS.hlsl` |

---

## 既存コードの整合性確認

### ✅ 準拠している点

1. **クラス名**: `Application`, `GraphicsDevice`, `RenderSystem` など適切
2. **メンバ変数**: `m_device`, `m_isInitialized` など適切
3. **定数**: `kBackBufferFormat`, `kDepthFormat` など適切
4. **bool関数**: `IsEnabled()`, `IsInitialized()` など適切
5. **ファイル名**: 全てUpperCamelCase

### 🔧 改善が必要な点

#### 1. 一部の構造体メンバ

```cpp
// 現状（RenderItem）
struct RenderItem final {
    Mesh* m_mesh = nullptr;      // 構造体なのに m_ プレフィックス
    Material* m_material = nullptr;
    // ...
};

// 推奨（構造体はプレフィックスなし or 統一）
struct RenderItem final {
    Mesh*       mesh     = nullptr;
    Material*   material = nullptr;
    // または m_ を維持して統一感を保つ
};
```

**判断:** 既存の `m_` プレフィックスを維持して一貫性を保つ

#### 2. コメントの言語統一

```cpp
// 現状（混在）
bool m_isInitialized = false;    // 初期化済みフラグ
bool m_isQuitRequested = false;  // 終了リクエストフラグ

// 推奨（日本語で統一、または英語で統一）
```

**判断:** 日本語コメントで統一（現在の方針を維持）

---

## Doxygen形式コメント規約

### 基本形式

```cpp
/// @brief  関数の簡易的な説明
/// @param  paramName パラメータの説明
/// @return 返り値の説明（返り値がある場合）
/// @note   詳細な説明（必要な場合）
```

### 列揃えルール

`@xxx` の後の最初の文字列の列を揃える：

```cpp
/// @brief  Componentを追加して返す
/// @param  args    コンストラクタ引数
/// @return 追加したComponentのポインタ
/// @note   所有権はEntityが持つ
template <ComponentDerived T, class... Args>
T* AddComponent(Args&&... args);
```

### クラスドキュメント

```cpp
/**
 * @brief Entity群を管理し、Update/Drawを統括する
 *
 * - Update/LateUpdate/Draw を Entity→Component へ伝搬
 * - 破棄の安全管理（遅延破棄）も担当
 */
class World final {
```

### ファイルヘッダー（推奨）

```cpp
/**
 * @file   World.h
 * @brief  Entity群を管理するWorldクラスの定義
 * @author [Author Name]
 * @date   YYYY-MM-DD
 */
#pragma once
```

---

## 現状コードの改善例

### 改善前（Entity.h）

```cpp
/**
 * @brief Component��ǉ����ĕԂ��i���L��Entity�j
 */
template <class T, class... Args>
T* AddComponent(Args&&... args) {
```

### 改善後

```cpp
/// @brief  Componentを追加して返す
/// @tparam T       追加するComponent型（Component派生であること）
/// @param  args    Componentのコンストラクタ引数
/// @return 追加したComponentのポインタ（所有権はEntityが持つ）
/// @note   追加時にOnAwake()が呼ばれ、Entityが開始済みならOnStart()も呼ばれる
template <ComponentDerived T, class... Args>
[[nodiscard]] T* AddComponent(Args&&... args);
```

---

## const/constexpr ルール

### 関数const

```cpp
// 状態を変えない関数は必ず const
int GetHp() const;
bool IsEnabled() const;
const Vector3& GetPosition() const;
```

### 引数const

```cpp
// 小さい型（int/float/bool/enum/ポインタ）: 値渡し
void Update(float deltaTime);
void SetEnabled(bool isEnabled);
void SetDevice(ID3D11Device* device);

// 大きい型（string/vector/Matrix）: const&
void SetMatrix(const DirectX::XMFLOAT4X4& matrix);
bool Load(const std::wstring& path);
```

### constexpr

```cpp
// コンパイル時定数
inline constexpr float kPi = 3.14159265f;
inline constexpr int kMaxComponents = 64;

// constexpr関数
[[nodiscard]] constexpr float ToRadians(float degrees) noexcept {
    return degrees * kPi / 180.0f;
}
```

---

## noexcept ルール

### 適用すべき関数

```cpp
// デストラクタ（暗黙的にnoexceptだが明示推奨）
~Component() noexcept = default;

// ゲッター（例外を投げない）
[[nodiscard]] bool IsEnabled() const noexcept;
[[nodiscard]] Entity* GetOwner() const noexcept;

// 単純な状態変更
void Enable() noexcept;
void Disable() noexcept;

// ムーブ操作
Entity(Entity&&) noexcept = default;
Entity& operator=(Entity&&) noexcept = default;
```

### 適用しない関数

```cpp
// 失敗の可能性がある初期化
bool Initialize(const Settings& settings);

// リソースロード
bool Load(const std::wstring& path);

// メモリ確保を伴う操作
Entity* CreateEntity();
```

---

## [[nodiscard]] ルール

### 適用すべき関数

```cpp
// ファクトリ関数
[[nodiscard]] Entity* CreateEntity();
[[nodiscard]] T* AddComponent();

// ゲッター
[[nodiscard]] bool IsEnabled() const noexcept;
[[nodiscard]] const Vector3& GetPosition() const;

// 初期化結果
[[nodiscard]] bool Initialize(const Settings& settings);
[[nodiscard]] bool Load(const std::wstring& path);
```

### 適用しない関数

```cpp
// 副作用が主目的
void Update(float deltaTime);
void Draw();
void Finalize();
```

---

## インクルード順序

```cpp
// 1. 対応するヘッダー（.cppの場合）
#include "World.h"

// 2. 標準ライブラリ
#include <vector>
#include <memory>
#include <string>

// 3. Windowsヘッダー
#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>

// 4. サードパーティ
#include <DirectXMath.h>

// 5. プロジェクト内ヘッダー（Engine）
#include "Engine/Core/Logger.h"
#include "Engine/Scene/Entity.h"

// 6. プロジェクト内ヘッダー（Game）
#include "Game/Scripts/PlayerController.h"
```

---

## フォーマット規約

### インデント

- **スペース4つ**（タブではなく）

### 波括弧

```cpp
// 関数: 次の行
void Function()
{
    // ...
}

// または同じ行（プロジェクト内で統一）
void Function() {
    // ...
}

// 現状は「次の行」スタイルが混在しているため統一推奨
```

### 行の長さ

- **120文字**を目安に折り返し

### 空行

```cpp
class Example {
public:
    // public メンバー

private:
    // private メンバー
};  // クラス終了後に空行

// 次のクラスまたは関数
```

---

## 改善チェックリスト

### 即時適用可能

- [ ] `[[nodiscard]]` の追加（ファクトリ/ゲッター）
- [ ] `noexcept` の追加（単純ゲッター/セッター）
- [ ] コメントの言語統一
- [ ] Doxygenコメントの列揃え

### 中期適用

- [ ] 文字化けコメントの修正
- [ ] インクルード順序の統一
- [ ] フォーマットの統一

### 長期適用

- [ ] ファイルヘッダーコメントの追加
- [ ] 詳細なAPIドキュメント作成
- [ ] Doxygenによるドキュメント生成設定

---

## ツール推奨

### clang-format

`.clang-format` 設定ファイル例：

```yaml
---
Language: Cpp
BasedOnStyle: Microsoft
IndentWidth: 4
TabWidth: 4
UseTab: Never
ColumnLimit: 120
AccessModifierOffset: -4
AlignConsecutiveAssignments: true
AlignConsecutiveDeclarations: true
AllowShortFunctionsOnASingleLine: Inline
BreakBeforeBraces: Allman
PointerAlignment: Left
---
```

### clang-tidy

推奨チェック：
- `modernize-use-override`
- `modernize-use-nodiscard`
- `readability-identifier-naming`
- `cppcoreguidelines-*`

---

## 次のステップ

1. このドキュメントをチームで共有
2. 即時適用可能な項目から着手
3. 新規コードはこの規約に従う
4. 既存コードは段階的に改善
