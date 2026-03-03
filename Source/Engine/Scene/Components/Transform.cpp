#include "Transform.h"

#include <algorithm> // std::max
#include <cmath>

namespace Engine {

    namespace {
		// �X�P�[���̍ŏ��l�i0�ȉ���h�����߁j
        constexpr float kMinScale = 0.0001f;
    }


    float Transform::NormalizeAngle(float angle){
		// 0�`360�x�͈̔͂ɐ��K��
		angle = std::fmodf(angle, 360.0f);
		// ���̒l�̏ꍇ��360�x�����Z
        if (angle < 0.0f) {
            angle += 360.0f;
        }
		return angle;
    }

    Vector3 Transform::NormalizeAngle(const Vector3& angles){
        return Vector3(
            NormalizeAngle(angles.x),
            NormalizeAngle(angles.y),
            NormalizeAngle(angles.z)
		);
    }


    void Transform::AddChild(Transform* child){
		m_children.push_back(child);
    }

    void Transform::RemoveChild(Transform* child){
		m_children.remove(child);
    }


    void Transform::SetPosition(const Vector3& position) {
        m_position = position;
        MarkDirty();
    }

    void Transform::SetPosition(float x, float y, float z) {
		SetPosition(Vector3(x, y, z));
    }

    void Transform::AddPosition(const Vector3& delta) {
        m_position += delta;
        MarkDirty();
    }

    void Transform::AddPosition(float deltaX, float deltaY, float deltaZ){
		AddPosition(Vector3(deltaX, deltaY, deltaZ));
    }

    const Vector3& Transform::GetPosition() const {
        return m_position;
    }

    void Transform::SetWorldPosition(const Vector3& worldPosition){
        if (m_parent) {
            // 親のワールド行列の逆行列を使って、ローカル座標に変換
            using namespace DirectX;
            const XMFLOAT4X4& parentWorld = m_parent->GetWorldMatrix();
            XMMATRIX parentMat = XMLoadFloat4x4(&parentWorld);
            
            // 行列式をチェックして可逆かどうか確認
            XMVECTOR det;
            XMMATRIX parentInv = XMMatrixInverse(&det, parentMat);
            
            // 行列式が0に近い場合は、直接ワールド座標を設定
            if (XMVectorGetX(XMVectorAbs(det)) < 0.0001f) {
                m_position = worldPosition;
            } else {
                XMVECTOR worldPos = worldPosition.ToXMVECTOR(1.0f);
                XMVECTOR localPos = XMVector3TransformCoord(worldPos, parentInv);
                m_position = Vector3(localPos);
            }
        } else {
            m_position = worldPosition;
        }
        MarkDirty();
    }

    void Transform::SetWorldPosition(float x, float y, float z){
        SetWorldPosition(Vector3(x, y, z));
    }

    void Transform::AddWorldPosition(const Vector3& deltaWorldPosition){
        Vector3 currentWorldPos = GetWorldPosition();
        SetWorldPosition(currentWorldPos + deltaWorldPosition);
    }

    const Vector3 Transform::GetWorldPosition() const{
        using namespace DirectX;
        RebuildWorldIfDirty();
        // ワールド行列の位置成分を抽出
        return Vector3(m_localToWorldMatrix._41, m_localToWorldMatrix._42, m_localToWorldMatrix._43);
    }

    void Transform::SetParent(Transform* parent) {
        // 自身を親にすることは禁止（循環参照防止）
        if (parent == this) {
            return;
        }
        
        // 循環参照チェック（新しい親が自身の子孫でないことを確認）
        Transform* current = parent;
        while (current) {
            if (current == this) {
                return; // 循環参照が発生するため設定しない
            }
            current = current->m_parent;
        }
        
        // 既存の親から削除
        if (m_parent) {
            m_parent->RemoveChild(this);
        }
        
        m_parent = parent;
        
        // 新しい親に追加
        if (m_parent) {
            m_parent->AddChild(this);
        }
        
        MarkDirty();
    }

    Transform* Transform::GetParent() const {
        return m_parent;
    }

    const std::list<Transform*>& Transform::GetChildren() const {
        return m_children;
    }

    void Transform::SetRotationEulerDegrees(const Vector3& eulerDegrees) {
        m_rotation = eulerDegrees;
        MarkDirty();
    }

    void Transform::AddRotationEulerDegrees(const Vector3& deltaDegrees) {
        m_rotation += deltaDegrees;
        MarkDirty();
    }

    const Vector3& Transform::GetRotationEulerDegrees() const {
        return m_rotation;
    }

    void Transform::SetYawPitchRollDegrees(float yawDegrees, float pitchDegrees, float rollDegrees) {
        // ������ x=pitch, y=yaw, z=roll
        m_rotation.x = pitchDegrees;
        m_rotation.y = yawDegrees;
        m_rotation.z = rollDegrees;
        MarkDirty();
    }

    void Transform::AddYawPitchRollDegrees(float yawDeltaDegrees, float pitchDeltaDegrees, float rollDeltaDegrees) {
        m_rotation.x += pitchDeltaDegrees;
        m_rotation.y += yawDeltaDegrees;
        m_rotation.z += rollDeltaDegrees;
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

    void Transform::AddScale(const Vector3& deltaScale){
		m_scale.x = std::max(m_scale.x + deltaScale.x, kMinScale);
		m_scale.y = std::max(m_scale.y + deltaScale.y, kMinScale);
		m_scale.z = std::max(m_scale.z + deltaScale.z, kMinScale);
		MarkDirty();
    }

    const Vector3& Transform::GetScale() const {
        return m_scale;
    }

    const DirectX::XMFLOAT4X4& Transform::GetWorldMatrix() const {
        RebuildWorldIfDirty();
        return m_localToWorldMatrix;
    }

    Vector3 Transform::GetRight() const {
        using namespace DirectX;
        const XMVECTOR q = MakeRotationQuaternion(m_rotation);
        const XMVECTOR v = XMVector3Rotate(XMVectorSet(1, 0, 0, 0), q);
        return Vector3(v);
    }

    Vector3 Transform::GetUp() const {
        using namespace DirectX;
        const XMVECTOR q = MakeRotationQuaternion(m_rotation);
        const XMVECTOR v = XMVector3Rotate(XMVectorSet(0, 1, 0, 0), q);
        return Vector3(v);
    }

    Vector3 Transform::GetForward() const {
        using namespace DirectX;
        const XMVECTOR q = MakeRotationQuaternion(m_rotation);
        const XMVECTOR v = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), q);
        return Vector3(v);
    }

    void Transform::MarkDirty() {
        if (m_isDirty) return; // 既にdirtyなら子への伝播は不要
        m_isDirty = true;
        // 子にも伝播
        for (auto* child : m_children) {
            if (child) {
                child->MarkDirty();
            }
        }
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
        const XMVECTOR q = MakeRotationQuaternion(m_rotation);
        const XMMATRIX r = XMMatrixRotationQuaternion(q);
        const XMMATRIX t = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

        XMMATRIX local = s * r * t;
        
        // 親がいれば親のワールド行列と合成
        if (m_parent) {
            const XMFLOAT4X4& parentWorld = m_parent->GetWorldMatrix();
            XMMATRIX parentMat = XMLoadFloat4x4(&parentWorld);
            local = local * parentMat;
        }
        
        XMStoreFloat4x4(&m_localToWorldMatrix, local);

        m_isDirty = false;
    }

} // namespace Engine
