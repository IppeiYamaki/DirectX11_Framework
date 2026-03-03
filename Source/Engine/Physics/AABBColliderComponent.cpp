/// @file   AABBColliderComponent.cpp
/// @brief  AABB（軸並行境界ボックス）コライダーコンポーネント実装
#include "AABBColliderComponent.h"
#include "SphereColliderComponent.h"
#include "CapsuleColliderComponent.h"

#include "Engine/Scene/Components/Transform.h"
#include "Engine/Core/Logger.h"

#include <algorithm>
#include <cmath>

namespace Engine {

    AABBColliderComponent::AABBColliderComponent()
        : m_halfExtents(0.5f, 0.5f, 0.5f)
        , m_offset()
        , m_worldShape() {
        SetColliderType(ColliderType::AABB);
    }

    //============================================================
    // 形状設定
    //============================================================

    void AABBColliderComponent::SetHalfExtents(const Vector3& halfExtents) {
        m_halfExtents = halfExtents;
        UpdateWorldShape();
    }

    const Vector3& AABBColliderComponent::GetHalfExtents() const {
        return m_halfExtents;
    }

    void AABBColliderComponent::SetOffset(const Vector3& offset) {
        m_offset = offset;
        UpdateWorldShape();
    }

    const Vector3& AABBColliderComponent::GetOffset() const {
        return m_offset;
    }

    const AABBShape& AABBColliderComponent::GetWorldAABB() const {
        return m_worldShape;
    }

    //============================================================
    // オーバーライド
    //============================================================

    void AABBColliderComponent::UpdateWorldShape() {
        auto* transform = GetTransform();
        if (!transform) return;

        // ワールド位置を取得
        Vector3 worldPos = transform->GetWorldPosition();

        // スケールを考慮した半径を計算
        const Vector3& scale = transform->GetScale();
        Vector3 scaledHalfExtents(
            m_halfExtents.x * scale.x,
            m_halfExtents.y * scale.y,
            m_halfExtents.z * scale.z
        );

        // オフセットを適用した中心位置
        Vector3 center = worldPos + m_offset;

        // ワールドAABBを更新
        m_worldShape.SetFromCenterAndHalfExtents(center, scaledHalfExtents);
    }

    bool AABBColliderComponent::CheckCollision(ColliderComponent& other, CollisionManifold& outManifold) {
        outManifold.Reset();
        outManifold.m_colliderA = this;
        outManifold.m_colliderB = &other;

        switch (other.GetColliderType()) {
        case ColliderType::AABB:
            return CheckAABBvsAABB(static_cast<AABBColliderComponent&>(other), outManifold);
        case ColliderType::Sphere:
            return CheckAABBvsSphere(static_cast<SphereColliderComponent&>(other), outManifold);
        case ColliderType::Capsule:
            return CheckAABBvsCapsule(static_cast<CapsuleColliderComponent&>(other), outManifold);
        default:
            return false;
        }
    }

    //============================================================
    // 内部衝突検出
    //============================================================

    bool AABBColliderComponent::CheckAABBvsAABB(AABBColliderComponent& other, CollisionManifold& outManifold) {
        const AABBShape& a = m_worldShape;
        const AABBShape& b = other.GetWorldAABB();

        // 各軸で重なりをチェック
        if (a.m_max.x < b.m_min.x || a.m_min.x > b.m_max.x) return false;
        if (a.m_max.y < b.m_min.y || a.m_min.y > b.m_max.y) return false;
        if (a.m_max.z < b.m_min.z || a.m_min.z > b.m_max.z) return false;

        outManifold.m_hasCollision = true;

        // 各軸のめり込み量を計算
        float overlapX = (std::min)(a.m_max.x, b.m_max.x) - (std::max)(a.m_min.x, b.m_min.x);
        float overlapY = (std::min)(a.m_max.y, b.m_max.y) - (std::max)(a.m_min.y, b.m_min.y);
        float overlapZ = (std::min)(a.m_max.z, b.m_max.z) - (std::max)(a.m_min.z, b.m_min.z);

        // 最小のめり込み軸を選択（分離軸）
        Vector3 normal;
        float penetration;

        Vector3 centerA = a.GetCenter();
        Vector3 centerB = b.GetCenter();
        Vector3 diff = centerB - centerA;

        if (overlapX <= overlapY && overlapX <= overlapZ) {
            penetration = overlapX;
            normal = Vector3(diff.x >= 0 ? 1.0f : -1.0f, 0, 0);
        }
        else if (overlapY <= overlapX && overlapY <= overlapZ) {
            penetration = overlapY;
            normal = Vector3(0, diff.y >= 0 ? 1.0f : -1.0f, 0);
        }
        else {
            penetration = overlapZ;
            normal = Vector3(0, 0, diff.z >= 0 ? 1.0f : -1.0f);
        }

        // 接触点を設定
        outManifold.m_contact.m_normal = normal;
        outManifold.m_contact.m_penetration = penetration;
        outManifold.m_contact.m_point = (centerA + centerB) * 0.5f;

        return true;
    }

    bool AABBColliderComponent::CheckAABBvsSphere(SphereColliderComponent& other, CollisionManifold& outManifold) {
        const AABBShape& aabb = m_worldShape;
        const SphereShape& sphere = other.GetWorldSphere();

        // 最近接点を計算
        Vector3 closest;
        closest.x = (std::max)(aabb.m_min.x, (std::min)(sphere.m_center.x, aabb.m_max.x));
        closest.y = (std::max)(aabb.m_min.y, (std::min)(sphere.m_center.y, aabb.m_max.y));
        closest.z = (std::max)(aabb.m_min.z, (std::min)(sphere.m_center.z, aabb.m_max.z));

        // 距離を計算
        Vector3 diff = sphere.m_center - closest;
        float distSq = diff.LengthSquared();

        if (distSq > sphere.m_radius * sphere.m_radius) {
            return false;
        }

        outManifold.m_hasCollision = true;

        float dist = std::sqrt(distSq);
        if (dist > 0.0001f) {
            outManifold.m_contact.m_normal = diff * (1.0f / dist);
        }
        else {
            // 球の中心がAABB内にある場合
            outManifold.m_contact.m_normal = Vector3(0, 1, 0);
        }

        outManifold.m_contact.m_penetration = sphere.m_radius - dist;
        outManifold.m_contact.m_point = closest;

        return true;
    }

    bool AABBColliderComponent::CheckAABBvsCapsule(CapsuleColliderComponent& other, CollisionManifold& outManifold) {
        const AABBShape& aabb = m_worldShape;
        const CapsuleShape& capsule = other.GetWorldCapsule();

        // カプセルの線分上の最近接点を求め、その点に対する球vsAABBで判定
        Vector3 top = capsule.GetTopSphereCenter();
        Vector3 bottom = capsule.GetBottomSphereCenter();
        Vector3 lineDir = top - bottom;
        float lineLen = lineDir.Length();
        if (lineLen > 0.0001f) {
            lineDir = lineDir * (1.0f / lineLen);
        }

        // AABBの中心に最も近いカプセル線分上の点
        Vector3 aabbCenter = aabb.GetCenter();
        float t = Vector3::Dot(aabbCenter - bottom, lineDir);
        t = (std::max)(0.0f, (std::min)(t, lineLen));
        Vector3 closestOnLine = bottom + lineDir * t;

        // その点からAABBへの最近接点
        Vector3 closestOnAABB;
        closestOnAABB.x = (std::max)(aabb.m_min.x, (std::min)(closestOnLine.x, aabb.m_max.x));
        closestOnAABB.y = (std::max)(aabb.m_min.y, (std::min)(closestOnLine.y, aabb.m_max.y));
        closestOnAABB.z = (std::max)(aabb.m_min.z, (std::min)(closestOnLine.z, aabb.m_max.z));

        // 距離チェック
        Vector3 diff = closestOnLine - closestOnAABB;
        float distSq = diff.LengthSquared();

        if (distSq > capsule.m_radius * capsule.m_radius) {
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

        outManifold.m_contact.m_penetration = capsule.m_radius - dist;
        outManifold.m_contact.m_point = closestOnAABB;

        return true;
    }

} // namespace Engine
