#pragma once

#include <DirectXMath.h>

#include "Engine/Math/Vector3.h"
#include "Engine/Scene/Component.h"

namespace Engine {

    /**
     * @brief 位置・回転・拡縮を管理するComponent（Engine::Vector3版）
     *
     * - 値は private
     * - 意図が分かるAPI（TeleportTo / MoveBy など）
     * - ワールド行列は dirty + キャッシュ
     */
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
        //============================================================
        void SetRotationEulerDegrees(const Vector3& eulerDegrees);
        void RotateByEulerDegrees(const Vector3& deltaDegrees);

        const Vector3& GetRotationEulerDegrees() const;

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
        Vector3 m_position              { 0.0f, 0.0f, 0.0f };   // ワールド位置
        Vector3 m_rotationEulerDegrees  { 0.0f, 0.0f, 0.0f };   // オイラー角（度数法）
        Vector3 m_scale                 { 1.0f, 1.0f, 1.0f };   // ワールド拡縮

        mutable bool                m_isDirty           = true; // ワールド行列再構築要否
		mutable DirectX::XMFLOAT4X4 m_worldMatrix{};            // ワールド行列キャッシュ
    };

} // namespace Engine
