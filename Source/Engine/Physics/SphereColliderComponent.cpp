/// @file   SphereColliderComponent.cpp
/// @brief  球コライダーコンポーネント実装
#include "SphereColliderComponent.h"
#include "AABBColliderComponent.h"
#include "CapsuleColliderComponent.h"

#include "Engine/Scene/Components/Transform.h"

#include <algorithm>
#include <cmath>

namespace Engine {

    SphereColliderComponent::SphereColliderComponent()
        : m_radius(0.5f)
        , m_offset()
        , m_worldShape() {
        SetColliderType(ColliderType::Sphere);
    }

    //============================================================
    // 形状設定
    //============================================================

    void SphereColliderComponent::SetRadius(float radius) {
        m_radius = radius;
        UpdateWorldShape();
    }

    float SphereColliderComponent::GetRadius() const {
        return m_radius;
    }

    void SphereColliderComponent::SetOffset(const Vector3& offset) {
        m_offset = offset;
        UpdateWorldShape();
    }

    const Vector3& SphereColliderComponent::GetOffset() const {
        return m_offset;
    }

    const SphereShape& SphereColliderComponent::GetWorldSphere() const {
        return m_worldShape;
    }

    //============================================================
    // オーバーライド
    //============================================================

    void SphereColliderComponent::UpdateWorldShape() {
        auto* transform = GetTransform();
        if (!transform) return;

        // ワールド位置を取得
        Vector3 worldPos = transform->GetWorldPosition();

        // スケールを考慮した半径を計算（最大スケールを使用）
        const Vector3& scale = transform->GetScale();
        float maxScale = (std::max)({scale.x, scale.y, scale.z});

        m_worldShape.m_center = worldPos + m_offset;
        m_worldShape.m_radius = m_radius * maxScale;
    }

    bool SphereColliderComponent::CheckCollision(ColliderComponent& other, CollisionManifold& outManifold) {
        outManifold.Reset();
        outManifold.m_colliderA = this;
        outManifold.m_colliderB = &other;

        switch (other.GetColliderType()) {
        case ColliderType::Sphere:
            return CheckSpherevsSphere(static_cast<SphereColliderComponent&>(other), outManifold);
        case ColliderType::AABB:
            return CheckSpherevsAABB(static_cast<AABBColliderComponent&>(other), outManifold);
        case ColliderType::Capsule:
            return CheckSpherevsCapsule(static_cast<CapsuleColliderComponent&>(other), outManifold);
        default:
            return false;
        }
    }

    //============================================================
    // 内部衝突検出
    //============================================================

    bool SphereColliderComponent::CheckSpherevsSphere(SphereColliderComponent& other, CollisionManifold& outManifold) {
        const SphereShape& a = m_worldShape;
        const SphereShape& b = other.GetWorldSphere();

        Vector3 diff = b.m_center - a.m_center;
        float distSq = diff.LengthSquared();
        float radiusSum = a.m_radius + b.m_radius;

        if (distSq > radiusSum * radiusSum) {
            return false;
        }

        outManifold.m_hasCollision = true;

        float dist = std::sqrt(distSq);
        if (dist > 0.0001f) {
            outManifold.m_contact.m_normal = diff * (1.0f / dist);
        }
        else {
            // 完全に重なっている場合
            outManifold.m_contact.m_normal = Vector3(0, 1, 0);
        }

        outManifold.m_contact.m_penetration = radiusSum - dist;
        outManifold.m_contact.m_point = a.m_center + outManifold.m_contact.m_normal * a.m_radius;

        return true;
    }

    bool SphereColliderComponent::CheckSpherevsAABB(AABBColliderComponent& other, CollisionManifold& outManifold) {
        // AABB側の実装を呼び出し、法線を反転
        bool result = other.CheckCollision(*this, outManifold);
        if (result) {
            outManifold.m_contact.m_normal = -outManifold.m_contact.m_normal;
            // colliderA/Bを入れ替え
            outManifold.m_colliderA = this;
            outManifold.m_colliderB = &other;
        }
        return result;
    }

    bool SphereColliderComponent::CheckSpherevsCapsule(CapsuleColliderComponent& other, CollisionManifold& outManifold) {
        const SphereShape& sphere = m_worldShape;
        const CapsuleShape& capsule = other.GetWorldCapsule();

        // カプセルの線分上で球の中心に最も近い点を求める
        Vector3 top = capsule.GetTopSphereCenter();
        Vector3 bottom = capsule.GetBottomSphereCenter();
        Vector3 lineDir = top - bottom;
        float lineLen = lineDir.Length();
        if (lineLen > 0.0001f) {
            lineDir = lineDir * (1.0f / lineLen);
        }

        float t = Vector3::Dot(sphere.m_center - bottom, lineDir);
        t = (std::max)(0.0f, (std::min)(t, lineLen));
        Vector3 closestOnLine = bottom + lineDir * t;

        // 球とその点との距離
        Vector3 diff = sphere.m_center - closestOnLine;
        float distSq = diff.LengthSquared();
        float radiusSum = sphere.m_radius + capsule.m_radius;

        if (distSq > radiusSum * radiusSum) {
            return false;
        }

        outManifold.m_hasCollision = true;

        float dist = std::sqrt(distSq);
        if (dist > 0.0001f) {
            outManifold.m_contact.m_normal = diff * (1.0f / dist);
        }
        else {
            outManifold.m_contact.m_normal = Vector3(0, 1, 0);
        }

        outManifold.m_contact.m_penetration = radiusSum - dist;
        outManifold.m_contact.m_point = closestOnLine + outManifold.m_contact.m_normal * capsule.m_radius;

        return true;
    }

} // namespace Engine
