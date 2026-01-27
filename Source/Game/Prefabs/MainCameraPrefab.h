#pragma once

#include <DirectXMath.h>

#include "Engine/Math/Vector3.h"

namespace Engine { class Entity; }
namespace Game { struct WorldContext; }

namespace Game {

    class MainCameraPrefab final {
    public:
        struct SpawnDesc final {
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

            SpawnDesc() = default;

            // ctx.Spawn<MainCameraPrefab>(pos, yaw, pitch) ���ł���`
            SpawnDesc(const Engine::Vector3& position,
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

        static Engine::Entity* Spawn(WorldContext& ctx, const SpawnDesc& desc);
    };

} // namespace Game
