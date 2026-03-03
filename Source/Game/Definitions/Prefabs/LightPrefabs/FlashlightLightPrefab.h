/// @file   FlashlightLightPrefab.h
/// @brief  懐中電灯（SpotLight）Prefab
#pragma once

#include "Engine/Math/Vector3.h"

namespace Engine {
    struct SceneContext;
    class GameObject;
}

namespace Game {

    /// @brief 懐中電灯用SpotLight Prefab
    /// @note  追従機能付きスポットライト（プレイヤーの持ち物として使用）
    /// @example auto flashlight = ctx.Spawn<FlashlightLightPrefab>(position, target);
    class FlashlightLightPrefab final {
    public:
        struct SpawnDesc final {
            Engine::Vector3 m_position{ 0.0f, 1.5f, 0.0f };    ///< 初期位置
            Engine::Vector3 m_direction{ 0.0f, 0.0f, 1.0f };   ///< 初期方向
            Engine::GameObject* m_followTarget = nullptr;       ///< 追従対象（nullの場合は固定）
            Engine::Vector3 m_followOffset{ 0.0f, 1.5f, 0.5f }; ///< 追従オフセット

            SpawnDesc() = default;

            explicit SpawnDesc(const Engine::Vector3& position)
                : m_position(position) {
            }

            SpawnDesc(const Engine::Vector3& position, Engine::GameObject* followTarget)
                : m_position(position)
                , m_followTarget(followTarget) {
            }

            SpawnDesc(Engine::GameObject* followTarget, const Engine::Vector3& offset)
                : m_followTarget(followTarget)
                , m_followOffset(offset) {
            }
        };

        /// @brief 懐中電灯をスポーン
        static Engine::GameObject* Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc);

        //============================================================
        // デフォルトパラメータ（懐中電灯向け）
        //============================================================
        static constexpr float kDefaultIntensity = 5.0f;
        static constexpr float kDefaultRange = 30.0f;
        static constexpr float kDefaultInnerAngle = 15.0f;
        static constexpr float kDefaultOuterAngle = 30.0f;
        static constexpr float kConstantAttenuation = 1.0f;
        static constexpr float kLinearAttenuation = 0.05f;
        static constexpr float kQuadraticAttenuation = 0.01f;

        /// @brief 懐中電灯の白色（LED風）
        static Engine::Vector3 GetDefaultColor() {
            return Engine::Vector3(1.0f, 1.0f, 0.95f);
        }
    };

} // namespace Game
