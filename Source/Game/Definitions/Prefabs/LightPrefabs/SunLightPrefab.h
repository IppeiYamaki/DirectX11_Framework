/// @file   SunLightPrefab.h
/// @brief  太陽光（DirectionalLight）Prefab
#pragma once

#include "Engine/Math/Vector3.h"

namespace Engine {
    struct SceneContext;
    class GameObject;
}

namespace Game {

    /// @brief 太陽光用DirectionalLight Prefab
    /// @note  日周変化（昼夜サイクル）機能付き
    /// @example auto sun = ctx.Spawn<SunLightPrefab>(enableCycle, cycleDuration);
    class SunLightPrefab final {
    public:
        struct SpawnDesc final {
            Engine::Vector3 m_position{ 0.0f, 0.0f, 0.0f };   /// カメラの初期位置
            Engine::Vector3 m_direction{ 0.3f, -1.0f, 0.2f };   /// 初期方向
            float m_initialTimeOfDay = 0.5f;                    /// 初期時刻（0〜1、0.5=正午）
            bool m_enableCycle = false;                         /// 日周変化を有効にするか
            float m_cycleDuration = 120.0f;                     /// 1日のサイクル時間（秒）

            SpawnDesc() = default;

            explicit SpawnDesc(bool enableCycle, float cycleDuration = 120.0f)
                : m_enableCycle(enableCycle)
                , m_cycleDuration(cycleDuration) {
            }

            SpawnDesc(const Engine::Vector3& direction, float intensity = 1.0f)
                : m_direction(direction)
                , m_enableCycle(false) {
                (void)intensity;  // 将来の拡張用
            }
        };

        /// @brief 太陽光をスポーン
        static Engine::GameObject* Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc);

        //============================================================
        // デフォルトパラメータ（太陽光向け）
        //============================================================
        static constexpr float kDefaultIntensity = 1.5f;

        /// @brief 太陽光のデフォルト色（暖かい白色）
        static Engine::Vector3 GetDefaultColor() {
            return Engine::Vector3(1.0f, 1.0f, 0.95f);
        }

        /// @brief 太陽光のデフォルト環境光
        static Engine::Vector3 GetDefaultAmbient() {
            return Engine::Vector3(0.15f, 0.15f, 0.2f);
        }
    };

} // namespace Game
