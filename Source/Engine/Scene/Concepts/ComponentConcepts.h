#pragma once

/// @file   ComponentConcepts.h
/// @brief  Component型制約のためのC++20 Concepts定義

#include <concepts>
#include <type_traits>

namespace Engine {

    class Component;

    //============================================================
    // Component Concepts
    //============================================================

    /// @brief  Componentを継承しているかを検証するconcept
    /// @tparam T 検証対象の型
    template<typename T>
    concept ComponentDerived = std::is_base_of_v<Component, T>;

    /// @brief  Updateを実装しているComponentかを検証するconcept
    /// @tparam T 検証対象の型
    template<typename T>
    concept UpdatableComponent = ComponentDerived<T> && requires(T t, float dt) {
        { t.Update(dt) } -> std::same_as<void>;
    };

    /// @brief  Drawを実装しているComponentかを検証するconcept
    /// @tparam T 検証対象の型
    template<typename T>
    concept DrawableComponent = ComponentDerived<T> && requires(T t) {
        { t.Draw() } -> std::same_as<void>;
    };

    /// @brief  デフォルトコンストラクト可能なComponentかを検証するconcept
    /// @tparam T 検証対象の型
    template<typename T>
    concept DefaultConstructibleComponent = ComponentDerived<T> && std::default_initializable<T>;

    /// @brief  コピー不可なComponentかを検証するconcept（推奨される形）
    /// @tparam T 検証対象の型
    template<typename T>
    concept NonCopyableComponent = ComponentDerived<T> && 
        !std::is_copy_constructible_v<T> && 
        !std::is_copy_assignable_v<T>;

} // namespace Engine
