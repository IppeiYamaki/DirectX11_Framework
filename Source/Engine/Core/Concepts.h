/// @file   Concepts.h
/// @brief  C++20 Concepts for type constraints
#pragma once

#include <concepts>
#include <type_traits>

namespace Engine {

    // Forward declaration
    class Component;
    class GameObject;

    //============================================================
    // Component-related Concepts
    //============================================================

    /// @brief Componentの派生型であることを要求するConcept
    /// @tparam T 検査する型
    template <typename T>
    concept DerivedFromComponent = std::is_base_of_v<Component, T>;

    /// @brief Componentとして構築可能であることを要求するConcept
    /// @tparam T    コンポーネント型
    /// @tparam Args コンストラクタ引数型
    template <typename T, typename... Args>
    concept ComponentConstructible = DerivedFromComponent<T> &&
        std::is_constructible_v<T, Args...>;

    //============================================================
    // Lifecycle-related Concepts
    //============================================================

    /// @brief 初期化可能な型であることを要求するConcept
    /// @tparam T 検査する型
    template <typename T>
    concept Initializable = requires(T t) {
        { t.Initialize() } -> std::convertible_to<bool>;
        { t.Finalize() } -> std::same_as<void>;
    };

    /// @brief リセット可能な型であることを要求するConcept
    /// @tparam T 検査する型
    template <typename T>
    concept Resettable = requires(T t) {
        { t.Reset() } -> std::same_as<void>;
    };

    //============================================================
    // Update/Draw-related Concepts
    //============================================================

    /// @brief 更新可能な型であることを要求するConcept
    /// @tparam T 検査する型
    template <typename T>
    concept Updatable = requires(T t, float dt) {
        { t.Update(dt) } -> std::same_as<void>;
    };

    /// @brief 遅延更新可能な型であることを要求するConcept
    /// @tparam T 検査する型
    template <typename T>
    concept LateUpdatable = requires(T t, float dt) {
        { t.LateUpdate(dt) } -> std::same_as<void>;
    };

    /// @brief 描画可能な型であることを要求するConcept
    /// @tparam T 検査する型
    template <typename T>
    concept Drawable = requires(T t) {
        { t.Draw() } -> std::same_as<void>;
    };

    //============================================================
    // Enable/Disable-related Concepts
    //============================================================

    /// @brief 有効/無効切り替え可能な型であることを要求するConcept
    /// @tparam T 検査する型
    template <typename T>
    concept Enableable = requires(T t) {
        { t.Enable() } -> std::same_as<void>;
        { t.Disable() } -> std::same_as<void>;
        { t.IsEnabled() } -> std::convertible_to<bool>;
    };

    //============================================================
    // Resource-related Concepts
    //============================================================

    /// @brief ロード可能な型であることを要求するConcept
    /// @tparam T 検査する型
    template <typename T>
    concept Loadable = requires(T t) {
        { t.Load() } -> std::convertible_to<bool>;
        { t.Unload() } -> std::same_as<void>;
        { t.IsLoaded() } -> std::convertible_to<bool>;
    };

    //============================================================
    // Numeric Concepts
    //============================================================

    /// @brief 浮動小数点数型であることを要求するConcept
    /// @tparam T 検査する型
    template <typename T>
    concept FloatingPoint = std::is_floating_point_v<T>;

    /// @brief 数値型であることを要求するConcept
    /// @tparam T 検査する型
    template <typename T>
    concept Numeric = std::is_arithmetic_v<T>;

    //============================================================
    // Utility Concepts
    //============================================================

    /// @brief ポインタ型であることを要求するConcept
    /// @tparam T 検査する型
    template <typename T>
    concept Pointer = std::is_pointer_v<T>;

    /// @brief nullptrと比較可能な型であることを要求するConcept
    /// @tparam T 検査する型
    template <typename T>
    concept NullablePointer = requires(T t) {
        { t == nullptr } -> std::convertible_to<bool>;
        { t != nullptr } -> std::convertible_to<bool>;
    };

} // namespace Engine
