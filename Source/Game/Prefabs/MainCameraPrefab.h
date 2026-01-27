#pragma once

#include <DirectXMath.h>

#include "Engine/Math/Vector3.h"
#include "Engine/Scene/Prefab.h"

namespace Engine { class Entity; }
namespace Engine { struct SceneContext; }

namespace Game {

    struct MainCameraSpawnDesc final {
        Engine::Vector3 m_position{ 0, -5.0f, -8 };

        // ���Œ�J�����̊p�x�i�x�j
        float m_yawDeg = 0.0f;
        float m_pitchDeg = 20.0f; // �����������i����Ȃ���Ε������t�ɂ���OK�j
        float m_rollDeg = 0.0f;

        float m_fovYRad = DirectX::XM_PIDIV4;
        float m_aspect = 16.0f / 9.0f;
        float m_nearZ = 0.1f;
        float m_farZ = 1000.0f;

        bool m_isMain = true;

        MainCameraSpawnDesc() = default;

        // ctx.SpawnPrefab<MainCameraPrefab>(pos, yaw, pitch) ���ł���`
        MainCameraSpawnDesc(const Engine::Vector3& position,
            float yawDeg = 0.0f,
            float pitchDeg = 20.0f,
            float fovYRad = DirectX::XM_PIDIV4,
            float aspect = 16.0f / 9.0f,
            float nearZ = 0.1f,
            float farZ = 1000.0f,
            bool isMain = true)
            : m_position(position)
            , m_yawDeg(yawDeg)
            , m_pitchDeg(pitchDeg)
            , m_fovYRad(fovYRad)
            , m_aspect(aspect)
            , m_nearZ(nearZ)
            , m_farZ(farZ)
            , m_isMain(isMain) {
        }
    };

    class MainCameraPrefab final : public Engine::Prefab<MainCameraSpawnDesc> {
    public:
        using SpawnDesc = MainCameraSpawnDesc;

        Engine::Entity* Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc) override;
    };

} // namespace Game
