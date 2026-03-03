/// @file   LightPrefabs.h
/// @brief  ライト用Prefabクラス群（DirectionalLight, PointLight, SpotLight）
#pragma once

#include "Engine/Math/Vector3.h"
#include "Engine/Scene/SceneContext.h"

namespace Engine {
    class GameObject;
}

namespace Game {

    //============================================================
    // DirectionalLightPrefab
    //============================================================

    /// @brief 方向性ライト（太陽光、月光など）のPrefab
    /// @example auto sun = ctx.Spawn<DirectionalLightPrefab>(direction, color, intensity);
    class DirectionalLightPrefab final {
    public:
        struct SpawnDesc final {
            Engine::Vector3 m_direction{ 0.0f, -1.0f, 0.0f };  ///< 光の方向
            Engine::Vector3 m_color{ 1.0f, 1.0f, 1.0f };       ///< 光の色
            float m_intensity = 1.0f;                           ///< 光の強度
            Engine::Vector3 m_ambient{ 0.1f, 0.1f, 0.1f };     ///< 環境光

            SpawnDesc() = default;

            SpawnDesc(const Engine::Vector3& direction,
                      const Engine::Vector3& color = Engine::Vector3(1.0f, 1.0f, 1.0f),
                      float intensity = 1.0f,
                      const Engine::Vector3& ambient = Engine::Vector3(0.1f, 0.1f, 0.1f))
                : m_direction(direction)
                , m_color(color)
                , m_intensity(intensity)
                , m_ambient(ambient) {
            }
        };

        static Engine::GameObject* Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc);
    };

    //============================================================
    // PointLightPrefab
    //============================================================

    /// @brief 点光源（街灯、炎、光球など）のPrefab
    /// @example auto torch = ctx.Spawn<PointLightPrefab>(position, color, intensity, range);
    class PointLightPrefab final {
    public:
        struct SpawnDesc final {
            Engine::Vector3 m_position{ 0.0f, 0.0f, 0.0f };    ///< 光の位置
            Engine::Vector3 m_color{ 1.0f, 1.0f, 1.0f };       ///< 光の色
            float m_intensity = 1.0f;                           ///< 光の強度
            float m_range = 10.0f;                              ///< 到達距離
            float m_constantAttenuation = 1.0f;                 ///< 定数減衰
            float m_linearAttenuation = 0.09f;                  ///< 線形減衰
            float m_quadraticAttenuation = 0.032f;              ///< 二次減衰

            SpawnDesc() = default;

            SpawnDesc(const Engine::Vector3& position,
                      const Engine::Vector3& color = Engine::Vector3(1.0f, 1.0f, 1.0f),
                      float intensity = 1.0f,
                      float range = 10.0f)
                : m_position(position)
                , m_color(color)
                , m_intensity(intensity)
                , m_range(range) {
            }
        };

        static Engine::GameObject* Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc);
    };

    //============================================================
    // SpotLightPrefab
    //============================================================

    /// @brief スポットライト（懐中電灯、舞台照明など）のPrefab
    /// @example auto flashlight = ctx.Spawn<SpotLightPrefab>(position, direction, color, innerAngle, outerAngle);
    class SpotLightPrefab final {
    public:
        struct SpawnDesc final {
            Engine::Vector3 m_position{ 0.0f, 0.0f, 0.0f };    ///< 光の位置
            Engine::Vector3 m_direction{ 0.0f, -1.0f, 0.0f };  ///< 光の方向
            Engine::Vector3 m_color{ 1.0f, 1.0f, 1.0f };       ///< 光の色
            float m_intensity = 1.0f;                           ///< 光の強度
            float m_range = 10.0f;                              ///< 到達距離
            float m_innerAngle = 30.0f;                         ///< 内側コーン角度（度）
            float m_outerAngle = 45.0f;                         ///< 外側コーン角度（度）
            float m_constantAttenuation = 1.0f;                 ///< 定数減衰
            float m_linearAttenuation = 0.09f;                  ///< 線形減衰
            float m_quadraticAttenuation = 0.032f;              ///< 二次減衰

            SpawnDesc() = default;

            SpawnDesc(const Engine::Vector3& position,
                      const Engine::Vector3& direction = Engine::Vector3(0.0f, -1.0f, 0.0f),
                      const Engine::Vector3& color = Engine::Vector3(1.0f, 1.0f, 1.0f),
                      float intensity = 1.0f,
                      float innerAngle = 30.0f,
                      float outerAngle = 45.0f,
                      float range = 10.0f)
                : m_position(position)
                , m_direction(direction)
                , m_color(color)
                , m_intensity(intensity)
                , m_range(range)
                , m_innerAngle(innerAngle)
                , m_outerAngle(outerAngle) {
            }
        };

        static Engine::GameObject* Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc);
    };

} // namespace Game
