#include "Transform.h"

#include <algorithm> // std::max

namespace Engine {

    namespace {
        constexpr float kMinScale = 0.0001f;
    }

    void Transform::TeleportTo(const Vector3& position) {
        m_position = position;
        MarkDirty();
    }

    void Transform::MoveBy(const Vector3& delta) {
        m_position += delta;
        MarkDirty();
    }

    const Vector3& Transform::GetPosition() const {
        return m_position;
    }

    void Transform::SetRotationEulerDegrees(const Vector3& eulerDegrees) {
        m_rotationEulerDegrees = eulerDegrees;
        MarkDirty();
    }

    void Transform::RotateByEulerDegrees(const Vector3& deltaDegrees) {
        m_rotationEulerDegrees += deltaDegrees;
        MarkDirty();
    }

    const Vector3& Transform::GetRotationEulerDegrees() const {
        return m_rotationEulerDegrees;
    }

    // Åöí«â¡ÅFYaw/Pitch/Roll ÇÃï™Ç©ÇËÇ‚Ç∑Ç¢éwíË
    void Transform::SetYawPitchRollDegrees(float yawDegrees, float pitchDegrees, float rollDegrees) {
        // ì‡ïîÇÕ x=pitch, y=yaw, z=roll
        m_rotationEulerDegrees.x = pitchDegrees;
        m_rotationEulerDegrees.y = yawDegrees;
        m_rotationEulerDegrees.z = rollDegrees;
        MarkDirty();
    }

    void Transform::AddYawPitchRollDegrees(float yawDeltaDegrees, float pitchDeltaDegrees, float rollDeltaDegrees) {
        m_rotationEulerDegrees.x += pitchDeltaDegrees;
        m_rotationEulerDegrees.y += yawDeltaDegrees;
        m_rotationEulerDegrees.z += rollDeltaDegrees;
        MarkDirty();
    }

    void Transform::SetScale(const Vector3& scale) {
        m_scale.x = std::max(scale.x, kMinScale);
        m_scale.y = std::max(scale.y, kMinScale);
        m_scale.z = std::max(scale.z, kMinScale);
        MarkDirty();
    }

    void Transform::SetUniformScale(float uniformScale) {
        const float s = std::max(uniformScale, kMinScale);
        m_scale = Vector3(s, s, s);
        MarkDirty();
    }

    const Vector3& Transform::GetScale() const {
        return m_scale;
    }

    const DirectX::XMFLOAT4X4& Transform::GetWorldMatrix() const {
        RebuildWorldIfDirty();
        return m_worldMatrix;
    }

    Vector3 Transform::GetRight() const {
        using namespace DirectX;
        const XMVECTOR q = MakeRotationQuaternion(m_rotationEulerDegrees);
        const XMVECTOR v = XMVector3Rotate(XMVectorSet(1, 0, 0, 0), q);
        return Vector3(v);
    }

    Vector3 Transform::GetUp() const {
        using namespace DirectX;
        const XMVECTOR q = MakeRotationQuaternion(m_rotationEulerDegrees);
        const XMVECTOR v = XMVector3Rotate(XMVectorSet(0, 1, 0, 0), q);
        return Vector3(v);
    }

    Vector3 Transform::GetForward() const {
        using namespace DirectX;
        const XMVECTOR q = MakeRotationQuaternion(m_rotationEulerDegrees);
        const XMVECTOR v = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), q);
        return Vector3(v);
    }

    void Transform::MarkDirty() {
        m_isDirty = true;
    }

    DirectX::XMVECTOR Transform::MakeRotationQuaternion(const Vector3& eulerDegrees) {
        using namespace DirectX;

        const float pitch = XMConvertToRadians(eulerDegrees.x);
        const float yaw = XMConvertToRadians(eulerDegrees.y);
        const float roll = XMConvertToRadians(eulerDegrees.z);

        return XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
    }

    void Transform::RebuildWorldIfDirty() const {
        if (!m_isDirty) return;

        using namespace DirectX;

        const XMMATRIX s = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
        const XMVECTOR q = MakeRotationQuaternion(m_rotationEulerDegrees);
        const XMMATRIX r = XMMatrixRotationQuaternion(q);
        const XMMATRIX t = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

        const XMMATRIX world = s * r * t;
        XMStoreFloat4x4(&m_worldMatrix, world);

        m_isDirty = false;
    }

} // namespace Engine
