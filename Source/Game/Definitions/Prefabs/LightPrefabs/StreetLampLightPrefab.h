/// @file   StreetLampLightPrefab.h
/// @brief  街灯用ライトPrefab
#pragma once

#include "Engine/Math/Vector3.h"

namespace Engine {
    struct SceneContext;
    class GameObject;
}

namespace Game {

    /// @brief 街灯用ライトPrefab
    /// @note  暖色系のポイントライト、街灯のような雰囲気を演出
    /// @example auto streetLamp = ctx.Spawn<StreetLampLightPrefab>(position);
    class StreetLampLightPrefab final {
    public:
        struct SpawnDesc final {
            Engine::Vector3 m_position{ 0.0f, 5.0f, 0.0f };  ///< 街灯の位置（デフォルト：5m高）

            SpawnDesc() = default;

            explicit SpawnDesc(const Engine::Vector3& position)
                : m_position(position) {
            }
        };

        /// @brief 街灯ライトをスポーン
        static Engine::GameObject* Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc);

        //============================================================
        // デフォルトパラメータ（街灯向け）
        //============================================================
        static constexpr float kDefaultIntensity = 3.0f;
        static constexpr float kDefaultRange = 20.0f;
        static constexpr float kConstantAttenuation = 1.0f;
        static constexpr float kLinearAttenuation = 0.07f;
        static constexpr float kQuadraticAttenuation = 0.017f;

        /// @brief 街灯の暖色（ナトリウムランプ風）
        static Engine::Vector3 GetDefaultColor() {
            return Engine::Vector3(1.0f, 0.85f, 0.6f);
        }
    };

} // namespace Game
