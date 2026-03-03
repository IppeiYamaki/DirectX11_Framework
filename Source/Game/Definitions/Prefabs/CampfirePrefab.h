/// @file   CampfirePrefab.h
/// @brief  焚き火用アニメーションモデル Prefab
#pragma once

#include "Engine/Math/Vector3.h"

namespace Engine {
    struct SceneContext;
    class GameObject;
}

namespace Game {

    /// @brief 焚き火用Prefab
    /// @note  アニメーションモデル（焚き火）と、子オブジェクトとしてFireLightPrefab（炎・光源）を持つ
    /// @example auto campfire = ctx.Spawn<CampfirePrefab>(position);
    class CampfirePrefab final {
    public:
        struct SpawnDesc final {
            Engine::Vector3 m_position{ 0.0f, 0.0f, 0.0f };  ///< 焚き火の位置
            float m_uniformScale = 1.0f;                      ///< 均一スケール
            
            // FireFlickerパラメータ
            float m_fireHeight = 1.5f;                        ///< 炎の高さ（ローカルY座標）
            float m_intensityFlickerRange = 0.3f;             ///< 強度揺らぎ幅
            float m_positionFlickerRange = 0.1f;              ///< 位置揺らぎ幅
            float m_flickerSpeed = 8.0f;                      ///< 揺らぎ速度

            SpawnDesc() = default;

            explicit SpawnDesc(const Engine::Vector3& position)
                : m_position(position) {
            }

            SpawnDesc(const Engine::Vector3& position, float uniformScale)
                : m_position(position)
                , m_uniformScale(uniformScale) {
            }

            SpawnDesc(const Engine::Vector3& position, float uniformScale, float fireHeight)
                : m_position(position)
                , m_uniformScale(uniformScale)
                , m_fireHeight(fireHeight) {
            }
        };

        /// @brief 焚き火をスポーン
        /// @param ctx SceneContext
        /// @param desc 生成パラメータ
        /// @return 生成されたGameObject（親オブジェクト）
        static Engine::GameObject* Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc);

        //============================================================
        // デフォルトパラメータ
        //============================================================
        /// @brief デフォルトの炎色（オレンジ）
        static Engine::Vector3 GetDefaultFireColor() {
            return Engine::Vector3(1.0f, 0.6f, 0.2f);
        }

        /// @brief デフォルトの光の強度
        static constexpr float kDefaultIntensity = 2.5f;
        
        /// @brief デフォルトの光の範囲
        static constexpr float kDefaultRange = 15.0f;
    };

} // namespace Game
