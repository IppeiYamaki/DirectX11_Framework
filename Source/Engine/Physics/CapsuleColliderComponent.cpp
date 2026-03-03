/// @file   CapsuleColliderComponent.cpp
/// @brief  カプセルコライダーコンポーネント実装
#include "CapsuleColliderComponent.h"
#include "SphereColliderComponent.h"
#include "AABBColliderComponent.h"

#include "Engine/Scene/Components/Transform.h"

#include <algorithm>
#include <cmath>

namespace Engine {

    CapsuleColliderComponent::CapsuleColliderComponent()
        : m_radius(0.5f)
        , m_height(1.0f)
        , m_offset()
        , m_worldShape() {
        SetColliderType(ColliderType::Capsule);
    }

    //============================================================
    // 形状設定
    //============================================================

    void CapsuleColliderComponent::SetRadius(float radius) {
        m_radius = radius;
        UpdateWorldShape();
    }

    float CapsuleColliderComponent::GetRadius() const {
        return m_radius;
    }

    void CapsuleColliderComponent::SetHeight(float height) {
        m_height = height;
        UpdateWorldShape();
    }

    float CapsuleColliderComponent::GetHeight() const {
        return m_height;
    }

    void CapsuleColliderComponent::SetOffset(const Vector3& offset) {
        m_offset = offset;
        UpdateWorldShape();
    }

    const Vector3& CapsuleColliderComponent::GetOffset() const {
        return m_offset;
    }

    const CapsuleShape& CapsuleColliderComponent::GetWorldCapsule() const {
        return m_worldShape;
    }

    //============================================================
    // オーバーライド
    //============================================================

    void CapsuleColliderComponent::UpdateWorldShape() {
        auto* transform = GetTransform();
        if (!transform) return;

        // ワールド位置を取得
        Vector3 worldPos = transform->GetWorldPosition();

        // スケールを考慮
        const Vector3& scale = transform->GetScale();
        float horizontalScale = (std::max)(scale.x, scale.z);

        m_worldShape.m_center = worldPos + m_offset;
        m_worldShape.m_radius = m_radius * horizontalScale;
        m_worldShape.m_height = m_height * scale.y;
    }

    bool CapsuleColliderComponent::CheckCollision(ColliderComponent& other, CollisionManifold& outManifold) {
        outManifold.Reset();
        outManifold.m_colliderA = this;
        outManifold.m_colliderB = &other;

        switch (other.GetColliderType()) {
        case ColliderType::Capsule:
            return CheckCapsulevsCapsule(static_cast<CapsuleColliderComponent&>(other), outManifold);
        case ColliderType::Sphere:
            return CheckCapsulevsSphere(static_cast<SphereColliderComponent&>(other), outManifold);
        case ColliderType::AABB:
            return CheckCapsulevsAABB(static_cast<AABBColliderComponent&>(other), outManifold);
        default:
            return false;
        }
    }

    //============================================================
    // 内部衝突検出
    //============================================================

    namespace {
        // 2つの線分間の最短距離の点を求める
        void ClosestPointsOnSegments(
            const Vector3& a1, const Vector3& a2,
            const Vector3& b1, const Vector3& b2,
            Vector3& outA, Vector3& outB
        ) {
            Vector3 da = a2 - a1;
            Vector3 db = b2 - b1;
            Vector3 r = a1 - b1;

            float a = Vector3::Dot(da, da);
            float e = Vector3::Dot(db, db);
            float f = Vector3::Dot(db, r);

            float s = 0.0f, t = 0.0f;

            if (a <= 0.0001f && e <= 0.0001f) {
                // 両方点
                s = t = 0.0f;
            }
            else if (a <= 0.0001f) {
                // aが点
                s = 0.0f;
                t = std::clamp(f / e, 0.0f, 1.0f);
            }
            else {
                float c = Vector3::Dot(da, r);
                if (e <= 0.0001f) {
                    // bが点
                    t = 0.0f;
                    s = std::clamp(-c / a, 0.0f, 1.0f);
                }
                else {
                    float b = Vector3::Dot(da, db);
                    float denom = a * e - b * b;

                    if (denom != 0.0f) {
                        s = std::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
                    }
                    else {
                        s = 0.0f;
                    }

                    t = (b * s + f) / e;

                    if (t < 0.0f) {
                        t = 0.0f;
                        s = std::clamp(-c / a, 0.0f, 1.0f);
                    }
                    else if (t > 1.0f) {
                        t = 1.0f;
                        s = std::clamp((b - c) / a, 0.0f, 1.0f);
                    }
                }
            }

            outA = a1 + da * s;
            outB = b1 + db * t;
        }
    }

    bool CapsuleColliderComponent::CheckCapsulevsCapsule(CapsuleColliderComponent& other, CollisionManifold& outManifold) {
        const CapsuleShape& a = m_worldShape;
        const CapsuleShape& b = other.GetWorldCapsule();

        // 各カプセルの線分端点
        Vector3 a1 = a.GetBottomSphereCenter();
        Vector3 a2 = a.GetTopSphereCenter();
        Vector3 b1 = b.GetBottomSphereCenter();
        Vector3 b2 = b.GetTopSphereCenter();

        // 線分間の最近接点を求める
        Vector3 closestA, closestB;
        ClosestPointsOnSegments(a1, a2, b1, b2, closestA, closestB);

        Vector3 diff = closestB - closestA;
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
            outManifold.m_contact.m_normal = Vector3(0, 1, 0);
        }

        outManifold.m_contact.m_penetration = radiusSum - dist;
        outManifold.m_contact.m_point = closestA + outManifold.m_contact.m_normal * a.m_radius;

        return true;
    }

    bool CapsuleColliderComponent::CheckCapsulevsSphere(SphereColliderComponent& other, CollisionManifold& outManifold) {
        // Sphere側の実装を呼び出し、法線を反転
        bool result = other.CheckCollision(*this, outManifold);
        if (result) {
            outManifold.m_contact.m_normal = -outManifold.m_contact.m_normal;
            outManifold.m_colliderA = this;
            outManifold.m_colliderB = &other;
        }
        return result;
    }

    bool CapsuleColliderComponent::CheckCapsulevsAABB(AABBColliderComponent& other, CollisionManifold& outManifold) {
        // AABB側の実装を呼び出し、法線を反転
        bool result = other.CheckCollision(*this, outManifold);
        if (result) {
            outManifold.m_contact.m_normal = -outManifold.m_contact.m_normal;
            outManifold.m_colliderA = this;
            outManifold.m_colliderB = &other;
        }
        return result;
    }

} // namespace Engine
