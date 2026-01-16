#pragma once

#include <DirectXMath.h>

#include "Engine/Math/Vector3.h"
#include "Engine/Scene/Component.h"

namespace Engine {

    class Transform final : public Component {
    public:
        Transform() = default;
        ~Transform() override = default;

        Transform(const Transform&) = delete;
        Transform& operator=(const Transform&) = delete;

        //============================================================
        // Position
        //============================================================
        void TeleportTo(const Vector3& position);
        void MoveBy(const Vector3& delta);

        const Vector3& GetPosition() const;

        //============================================================
        // Rotation (Euler degrees)
        // x = pitch, y = yaw, z = roll
        //============================================================
        void SetRotationEulerDegrees(const Vector3& eulerDegrees);
        void RotateByEulerDegrees(const Vector3& deltaDegrees);

        const Vector3& GetRotationEulerDegrees() const;

        // ★カメラ向け：Yaw/Pitch/Roll を分かりやすく指定
        void SetYawPitchRollDegrees(float yawDegrees, float pitchDegrees, float rollDegrees = 0.0f);
        void AddYawPitchRollDegrees(float yawDeltaDegrees, float pitchDeltaDegrees, float rollDeltaDegrees = 0.0f);

        //============================================================
        // Scale
        //============================================================
        void SetScale(const Vector3& scale);
        void SetUniformScale(float uniformScale);

        const Vector3& GetScale() const;

        //============================================================
        // Matrix / Direction
        //============================================================
        const DirectX::XMFLOAT4X4& GetWorldMatrix() const;

        Vector3 GetRight() const;
        Vector3 GetUp() const;
        Vector3 GetForward() const;

    private:
        void MarkDirty();
        void RebuildWorldIfDirty() const;

        static DirectX::XMVECTOR MakeRotationQuaternion(const Vector3& eulerDegrees);

    private:
        Vector3 m_position{ 0.0f, 0.0f, 0.0f };
        Vector3 m_rotationEulerDegrees{ 0.0f, 0.0f, 0.0f }; // x=pitch, y=yaw, z=roll（度）
        Vector3 m_scale{ 1.0f, 1.0f, 1.0f };

        mutable bool                m_isDirty = true;
        mutable DirectX::XMFLOAT4X4 m_worldMatrix{};
    };

} // namespace Engine
