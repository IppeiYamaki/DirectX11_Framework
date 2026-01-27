# DirectX11_Framework 開発ロードマップ

## 概要

このドキュメントでは、DirectX11_Frameworkの改善計画を短期・中期・長期の3段階に分けて説明します。

---

## 🎯 短期計画（1-2週間）

### 1. C++20 モダナイゼーション - 基礎

| タスク | 優先度 | 工数 | 詳細 |
|--------|--------|------|------|
| Concepts導入（Component系） | 高 | 2日 | `AddComponent<T>` のテンプレート制約をconcepts化 |
| range-based loops最適化 | 中 | 1日 | イテレーター使用箇所をranges APIで改善 |
| `std::span` 導入 | 中 | 1日 | 配列ポインタ引数を `std::span` に置換 |
| `constexpr` 拡充 | 低 | 0.5日 | 定数計算を `constexpr` 化 |

#### サンプル実装：Component Concepts

```cpp
// Engine/Scene/ComponentConcepts.h
#pragma once

#include <concepts>
#include <type_traits>

namespace Engine {

    class Component;

    /// @brief Componentを継承しているかを検証するconcept
    template<typename T>
    concept ComponentDerived = std::is_base_of_v<Component, T>;

    /// @brief Updateを実装しているComponentかを検証するconcept
    template<typename T>
    concept UpdatableComponent = ComponentDerived<T> && requires(T t, float dt) {
        { t.Update(dt) } -> std::same_as<void>;
    };

    /// @brief Drawを実装しているComponentかを検証するconcept
    template<typename T>
    concept DrawableComponent = ComponentDerived<T> && requires(T t) {
        { t.Draw() } -> std::same_as<void>;
    };

} // namespace Engine
```

### 2. コーディング規約の整備

| タスク | 優先度 | 工数 |
|--------|--------|------|
| Doxygen形式の一貫性チェック | 高 | 1日 |
| 命名規則違反の修正 | 中 | 1日 |
| ヘッダーガード → `#pragma once` 統一確認 | 低 | 0.5日 |

### 3. 即座に適用可能な最適化

| タスク | 優先度 | 工数 |
|--------|--------|------|
| `const` 適切性の見直し | 高 | 0.5日 |
| `noexcept` の追加 | 中 | 0.5日 |
| 不要なコピーの排除 | 中 | 1日 |

---

## 📅 中期計画（1-3ヶ月）

### 1. Entity+Component アーキテクチャ改善

| タスク | 優先度 | 工数 | 詳細 |
|--------|--------|------|------|
| ComponentPool導入 | 高 | 1週間 | メモリ局所性向上のためのプール化 |
| タグシステム実装 | 中 | 3日 | Entity検索の効率化 |
| イベントシステム | 中 | 1週間 | Component間の疎結合通信 |
| 遅延破棄システム | 高 | 3日 | Update中のDestroyEntityを安全化 |

#### アーキテクチャ改善の概要図

```
[現状]
World --> vector<unique_ptr<Entity>> --> vector<unique_ptr<Component>>
           ↓
         動的アロケーション多発、キャッシュ効率悪い

[改善後]
World --> ComponentPool<Transform>
      --> ComponentPool<MeshRenderer>
      --> ComponentPool<Camera>
      --> EntityRegistry（軽量ID管理）
           ↓
         連続メモリ配置、キャッシュフレンドリー
```

### 2. DirectX11 パフォーマンス最適化

| タスク | 優先度 | 工数 | 詳細 |
|--------|--------|------|------|
| RenderItem ソート最適化 | 高 | 3日 | シェーダー/マテリアル切替削減 |
| インスタンシング対応 | 高 | 1週間 | 同一メッシュのバッチ描画 |
| 定数バッファ管理改善 | 中 | 3日 | Map/Unmapの最適化 |
| リソースキャッシュ強化 | 中 | 3日 | テクスチャ/シェーダー参照管理 |

### 3. Sankou_00/Sankou_01 機能統合

| タスク | 優先度 | 工数 | 詳細 |
|--------|--------|------|------|
| AudioSource統合 | 中 | 1週間 | Sankou_00からの移植 |
| PostProcessシステム基盤 | 高 | 2週間 | Sankou_01を参考に実装 |
| Bloom/FXAA実装 | 中 | 1週間 | 基本的なポストエフェクト |

### 4. ステートパターン導入

| タスク | 優先度 | 工数 |
|--------|--------|------|
| StateBase/StateMachine実装 | 中 | 3日 |
| GameStateコンポーネント | 中 | 2日 |
| SceneManager改善 | 中 | 2日 |

---

## 🚀 長期計画（3-6ヶ月以上）

### 1. ECS完全移行

| タスク | 優先度 | 工数 | 詳細 |
|--------|--------|------|------|
| Archetype-based ECS設計 | 中 | 3週間 | 本格的なECSアーキテクチャ |
| SystemベースのUpdate | 中 | 2週間 | ComponentごとのSystemクラス |
| 並列処理対応 | 低 | 2週間 | System間の並列実行 |

### 2. 高度なレンダリング機能

| タスク | 優先度 | 工数 |
|--------|--------|------|
| デファードレンダリング | 中 | 3週間 |
| シャドウマッピング | 中 | 2週間 |
| PBR (Physically Based Rendering) | 低 | 3週間 |
| HDR / ToneMapping | 低 | 1週間 |

### 3. 開発ツール・デバッグ機能

| タスク | 優先度 | 工数 |
|--------|--------|------|
| ImGui統合 | 中 | 1週間 |
| パフォーマンスプロファイラー | 低 | 2週間 |
| シーンエディター（基本） | 低 | 1ヶ月 |

### 4. C++20/23 高度な機能活用

| タスク | 優先度 | 工数 |
|--------|--------|------|
| Modules対応検討 | 低 | 2週間 |
| Coroutines（非同期ロード） | 低 | 2週間 |
| `std::expected` 導入 | 低 | 1週間 |

---

## 優先度の判断基準

| 優先度 | 説明 |
|--------|------|
| **高** | 開発効率・パフォーマンスに大きく影響 / 既存の問題を解決 |
| **中** | 機能拡張として有用 / 将来のメンテナンス性向上 |
| **低** | Nice-to-have / 長期的な改善 |

---

## 実装順序の推奨

### Phase 1: 基盤整備（短期）
```
1. Concepts導入
2. コーディング規約整備
3. const/noexcept追加
```

### Phase 2: アーキテクチャ強化（中期）
```
4. 遅延破棄システム
5. ComponentPool
6. RenderItem最適化
7. イベントシステム
```

### Phase 3: 機能拡張（中期後半）
```
8. PostProcessシステム
9. インスタンシング
10. AudioSource統合
```

### Phase 4: 高度な機能（長期）
```
11. ECS完全移行
12. デファードレンダリング
13. 開発ツール
```

---

## 次のステップ

各提案の詳細については、以下のドキュメントを参照してください：

- [CPP20_MODERNIZATION.md](./CPP20_MODERNIZATION.md) - C++20活用の詳細
- [ARCHITECTURE_IMPROVEMENTS.md](./ARCHITECTURE_IMPROVEMENTS.md) - アーキテクチャ改善の詳細
- [DIRECTX11_OPTIMIZATION.md](./DIRECTX11_OPTIMIZATION.md) - DirectX11最適化の詳細
- [INTEGRATION_GUIDE.md](./INTEGRATION_GUIDE.md) - Sankou統合ガイド
- [CODING_STANDARDS.md](./CODING_STANDARDS.md) - コーディング規約
