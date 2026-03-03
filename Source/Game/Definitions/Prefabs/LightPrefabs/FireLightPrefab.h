/// @file   FireLightPrefab.h
/// @brief  炎のライトPrefab
#pragma once

#include "Engine/Math/Vector3.h"

namespace Engine {
    struct SceneContext;
    class GameObject;
}

namespace Game {

    /// @brief 炎用ライトPrefab
    /// @note  揺らぎ効果付きのポイントライト、たいまつや焚き火の演出に
    /// @example auto torch = ctx.Spawn<FireLightPrefab>(position);
    class FireLightPrefab final {
    public:
        struct SpawnDesc final {
            Engine::Vector3 m_position{ 0.0f, 1.5f, 0.0f };  ///< 炎の位置

            // 揺らぎパラメータ（オプション）
            float m_intensityFlickerRange = 0.3f;  ///< 強度揺らぎ幅
            float m_positionFlickerRange = 0.1f;   ///< 位置揺らぎ幅
            float m_flickerSpeed = 8.0f;           ///< 揺らぎ速度

            SpawnDesc() = default;

            explicit SpawnDesc(const Engine::Vector3& position)
                : m_position(position) {
            }

            SpawnDesc(const Engine::Vector3& position, float flickerSpeed)
                : m_position(position)
                , m_flickerSpeed(flickerSpeed) {
            }
        };

        /// @brief 炎ライトをスポーン
        static Engine::GameObject* Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc);

        //============================================================
        // デフォルトパラメータ（炎向け）
        //============================================================
        static constexpr float kDefaultIntensity = 2.5f;
        static constexpr float kDefaultRange = 15.0f;
        static constexpr float kConstantAttenuation = 1.0f;
        static constexpr float kLinearAttenuation = 0.09f;
        static constexpr float kQuadraticAttenuation = 0.032f;

        /// @brief 炎のオレンジ色
        static Engine::Vector3 GetDefaultColor() {
            return Engine::Vector3(1.0f, 0.6f, 0.2f);
        }
    };

} // namespace Game
