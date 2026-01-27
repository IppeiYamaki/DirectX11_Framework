# C++20 モダナイゼーション提案

## 概要

DirectX11_FrameworkをC++20の機能を活用してモダナイズするための具体的な提案と実装例を示します。

---

## 1. Concepts の活用

### 1.1 Component型制約

**現状のコード（Entity.h）:**
```cpp
template <class T, class... Args>
T* AddComponent(Args&&... args) {
    static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
    // ...
}
```

**改善案:**
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

    /// @brief デフォルトコンストラクト可能なComponentかを検証するconcept
    template<typename T>
    concept DefaultConstructibleComponent = ComponentDerived<T> && std::default_initializable<T>;

} // namespace Engine
```

**Entity.hでの使用:**
```cpp
#include "Engine/Scene/ComponentConcepts.h"

/// @brief Componentを追加して返す（所有はEntity）
template <ComponentDerived T, class... Args>
T* AddComponent(Args&&... args) {
    auto comp = std::make_unique<T>(std::forward<Args>(args)...);
    T* raw = comp.get();
    
    raw->SetOwner(this);
    m_components.emplace_back(std::move(comp));
    
    raw->OnAwake();
    
    if (m_hasStarted) {
        raw->OnStart();
    }
    
    return raw;
}

/// @brief 最初に見つかった T を返す（なければ nullptr）
template <ComponentDerived T>
T* GetComponent() {
    for (auto& c : m_components) {
        if (auto* t = dynamic_cast<T*>(c.get())) {
            return t;
        }
    }
    return nullptr;
}
```

### 1.2 GraphicsResource 型制約

**提案:**
```cpp
// Engine/Graphics/ResourceConcepts.h
#pragma once

#include <concepts>
#include <d3d11.h>
#include <wrl/client.h>

namespace Engine {

    /// @brief D3D11リソースとして使用可能な型を制約
    template<typename T>
    concept D3D11Resource = std::is_base_of_v<IUnknown, T>;

    /// @brief シェーダーリソースとして使用可能な型を制約
    template<typename T>
    concept ShaderResource = D3D11Resource<T> && requires {
        { T::GetShader() } -> D3D11Resource;
    };

    /// @brief ロード可能なアセット型を制約
    template<typename T>
    concept LoadableAsset = requires(T t, const std::wstring& path) {
        { t.IsLoaded() } -> std::same_as<bool>;
        { t.Unload() } -> std::same_as<void>;
    };

} // namespace Engine
```

---

## 2. Ranges の活用

### 2.1 Entity ループの改善

**現状:**
```cpp
void World::Update(float deltaTime) {
    if (!m_isInitialized) return;

    for (auto& e : m_entities) {
        e->UpdateComponents(deltaTime);
    }
}
```

**改善案（有効なEntityのみをフィルタ）:**
```cpp
#include <ranges>

void World::Update(float deltaTime) {
    if (!m_isInitialized) return;

    // 有効なEntityのみを更新
    auto enabledEntities = m_entities 
        | std::views::filter([](const auto& e) { return e->IsEnabled(); });
    
    for (auto& e : enabledEntities) {
        e->UpdateComponents(deltaTime);
    }
}
```

### 2.2 Component検索の改善

**現状（Entity.h）:**
```cpp
template <class T>
T* GetComponent() {
    static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
    for (auto& c : m_components) {
        if (auto* t = dynamic_cast<T*>(c.get())) {
            return t;
        }
    }
    return nullptr;
}
```

**改善案:**
```cpp
#include <ranges>
#include <algorithm>

/// @brief すべての T 型コンポーネントを返す
template <ComponentDerived T>
auto GetComponents() {
    return m_components 
        | std::views::transform([](const auto& c) { return dynamic_cast<T*>(c.get()); })
        | std::views::filter([](auto* ptr) { return ptr != nullptr; });
}

/// @brief 有効なコンポーネントのみを返す
auto GetEnabledComponents() {
    return m_components 
        | std::views::filter([](const auto& c) { return c->IsEnabled(); });
}
```

---

## 3. std::span の活用

### 3.1 配列ポインタの置き換え

**現状（GraphicsDevice.h）:**
```cpp
void Clear(const float clearColor[4]);
```

**改善案:**
```cpp
#include <span>

void Clear(std::span<const float, 4> clearColor);
```

### 3.2 頂点データの受け渡し

**現状:**
```cpp
bool CreateVertexBuffer(const void* vertices, size_t vertexCount, size_t stride);
```

**改善案:**
```cpp
template<typename T>
bool CreateVertexBuffer(std::span<const T> vertices) {
    // 型安全、サイズ情報を保持
    return CreateVertexBufferInternal(
        vertices.data(),
        vertices.size(),
        sizeof(T)
    );
}
```

---

## 4. constexpr の拡充

### 4.1 数学定数

**現状（散在）:**
```cpp
static const float kPi = 3.14159265f;
```

**改善案:**
```cpp
// Engine/Math/MathConstants.h
#pragma once

#include <numbers>

namespace Engine::Math {

    /// @brief 円周率
    inline constexpr float kPi = std::numbers::pi_v<float>;
    
    /// @brief 2π
    inline constexpr float kTwoPi = kPi * 2.0f;
    
    /// @brief π/2
    inline constexpr float kHalfPi = kPi / 2.0f;
    
    /// @brief ラジアン変換係数
    inline constexpr float kDegToRad = kPi / 180.0f;
    
    /// @brief 度変換係数
    inline constexpr float kRadToDeg = 180.0f / kPi;

    /// @brief 度からラジアンへの変換
    [[nodiscard]] constexpr float ToRadians(float degrees) noexcept {
        return degrees * kDegToRad;
    }

    /// @brief ラジアンから度への変換
    [[nodiscard]] constexpr float ToDegrees(float radians) noexcept {
        return radians * kRadToDeg;
    }

} // namespace Engine::Math
```

### 4.2 RenderState フラグ

**現状（RenderSystem.h）:**
```cpp
static constexpr std::uint32_t kRenderStateNone = 0;
```

**改善案（enum class + constexpr関数）:**
```cpp
// Engine/Graphics/RenderStateFlags.h
#pragma once

#include <cstdint>
#include <type_traits>

namespace Engine {

    /// @brief レンダリング状態フラグ
    enum class RenderStateFlag : std::uint32_t {
        None            = 0,
        DepthWriteOff   = 1 << 0,
        DepthTestOff    = 1 << 1,
        CullFront       = 1 << 2,
        CullNone        = 1 << 3,
        AlphaBlend      = 1 << 4,
    };

    /// @brief フラグのビット演算をサポート
    [[nodiscard]] constexpr RenderStateFlag operator|(RenderStateFlag a, RenderStateFlag b) noexcept {
        return static_cast<RenderStateFlag>(
            static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b)
        );
    }

    [[nodiscard]] constexpr RenderStateFlag operator&(RenderStateFlag a, RenderStateFlag b) noexcept {
        return static_cast<RenderStateFlag>(
            static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b)
        );
    }

    [[nodiscard]] constexpr bool HasFlag(RenderStateFlag flags, RenderStateFlag flag) noexcept {
        return (static_cast<std::uint32_t>(flags) & static_cast<std::uint32_t>(flag)) != 0;
    }

} // namespace Engine
```

---

## 5. std::format の活用

### 5.1 ログ出力の改善

**現状（Logger.cpp）:**
```cpp
void Logger::Info(const std::string& message) {
    // sprintf または string連結
}
```

**改善案:**
```cpp
#include <format>
#include <source_location>

class Logger final {
public:
    /// @brief 情報ログを出力
    template<typename... Args>
    static void Info(std::format_string<Args...> fmt, Args&&... args) {
        const std::string message = std::format(fmt, std::forward<Args>(args)...);
        OutputMessage(LogLevel::Info, message);
    }

    /// @brief エラーログを出力（ソース位置情報付き）
    template<typename... Args>
    static void Error(
        std::format_string<Args...> fmt,
        Args&&... args,
        const std::source_location loc = std::source_location::current()
    ) {
        const std::string message = std::format(fmt, std::forward<Args>(args)...);
        const std::string fullMessage = std::format(
            "[{}:{}] {}", 
            loc.file_name(), 
            loc.line(), 
            message
        );
        OutputMessage(LogLevel::Error, fullMessage);
    }

private:
    enum class LogLevel { Info, Warning, Error };
    static void OutputMessage(LogLevel level, const std::string& message);
};

// 使用例
Logger::Info("Entity count: {}", world.GetEntityCount());
Logger::Error("Failed to load shader: {}", shaderPath);
```

---

## 6. [[nodiscard]] の活用

### 6.1 返り値の明示

**提案:**
```cpp
class Entity final {
public:
    /// @brief Componentを追加して返す
    template <ComponentDerived T, class... Args>
    [[nodiscard]] T* AddComponent(Args&&... args);

    /// @brief 最初に見つかった T を返す
    template <ComponentDerived T>
    [[nodiscard]] T* GetComponent();

    /// @brief 有効かどうかを返す
    [[nodiscard]] bool IsEnabled() const noexcept;
};

class World final {
public:
    /// @brief Entityを生成して返す
    [[nodiscard]] Entity* CreateEntity();

    /// @brief Entity数を返す
    [[nodiscard]] std::uint32_t GetEntityCount() const noexcept;

    /// @brief 初期化済みかどうかを返す
    [[nodiscard]] bool IsInitialized() const noexcept;
};
```

---

## 7. 段階的な導入計画

### Phase 1（即座に適用可能）
- `[[nodiscard]]` の追加
- `noexcept` の追加
- `constexpr` の拡充

### Phase 2（1-2週間）
- Concepts の導入
- `std::span` の導入
- `std::format` の導入（Logger）

### Phase 3（2-4週間）
- Ranges の活用
- 数学関数の `constexpr` 化
- RenderStateFlag enum class化

---

## 8. 互換性の注意点

| 機能 | MSVC バージョン | 注意点 |
|------|----------------|--------|
| Concepts | VS2019 16.3+ | 完全サポート |
| Ranges | VS2019 16.6+ | 一部機能制限あり |
| std::format | VS2019 16.10+ | 完全サポート |
| std::span | VS2019 16.0+ | 完全サポート |
| source_location | VS2019 16.6+ | 完全サポート |

現在のプロジェクト設定（v143, C++20）では、上記すべての機能が利用可能です。

---

## 次のステップ

1. [ROADMAP.md](./ROADMAP.md) で優先度を確認
2. Phase 1 の変更を適用
3. 単体テストで動作確認
4. Phase 2 へ進む
