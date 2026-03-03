/// @file   HeightfieldColliderComponent.cpp
/// @brief  ハイトフィールドコライダーコンポーネント実装
#include "HeightfieldColliderComponent.h"
#include "SphereColliderComponent.h"
#include "AABBColliderComponent.h"
#include "CapsuleColliderComponent.h"

#include "Engine/Scene/Components/Transform.h"
#include "Engine/Core/Logger.h"

#include <algorithm>
#include <cmath>

namespace Engine {

    HeightfieldColliderComponent::HeightfieldColliderComponent()
        : m_heightFunc(nullptr)
        , m_minX(-1000.0f)
        , m_maxX(1000.0f)
        , m_minZ(-1000.0f)
        , m_maxZ(1000.0f)
        , m_defaultHeight(0.0f)
        , m_sampleDelta(0.1f) {
        SetColliderType(ColliderType::Heightfield);
        
        // 地形は固定物なのでデフォルトで静的設定
        ColliderDesc desc = ColliderDesc::Static();
        SetDesc(desc);
    }

    //============================================================
    // 形状設定
    //============================================================

    void HeightfieldColliderComponent::SetHeightFunction(const HeightFunction& func) {
        m_heightFunc = func;
    }

    void HeightfieldColliderComponent::SetBounds(float minX, float maxX, float minZ, float maxZ) {
        m_minX = minX;
        m_maxX = maxX;
        m_minZ = minZ;
        m_maxZ = maxZ;
    }

    float HeightfieldColliderComponent::GetHeightAt(float x, float z) const {
        if (!m_heightFunc || !IsInBounds(x, z)) {
            return m_defaultHeight;
        }
        return m_heightFunc(x, z);
    }

    Vector3 HeightfieldColliderComponent::GetNormalAt(float x, float z) const {
        if (!m_heightFunc) {
            return Vector3(0, 1, 0);
        }

        // 中心差分で法線を計算
        float d = m_sampleDelta;
        float hL = GetHeightAt(x - d, z);
        float hR = GetHeightAt(x + d, z);
        float hD = GetHeightAt(x, z - d);
        float hU = GetHeightAt(x, z + d);

        // 勾配ベクトルから法線を計算
        Vector3 normal(
            (hL - hR) / (2.0f * d),
            1.0f,
            (hD - hU) / (2.0f * d)
        );

        return normal.Normalized();
    }

    void HeightfieldColliderComponent::SetDefaultHeight(float height) {
        m_defaultHeight = height;
    }

    //============================================================
    // 衝突判定用ヘルパー
    //============================================================

    bool HeightfieldColliderComponent::CheckSphereCollision(
        const Vector3& sphereCenter,
        float sphereRadius,
        CollisionManifold& outManifold
    ) {
        // 球の中心直下の地形高さを取得
        float groundHeight = GetHeightAt(sphereCenter.x, sphereCenter.z);
        float sphereBottom = sphereCenter.y - sphereRadius;

        if (sphereBottom >= groundHeight) {
            return false;
        }

        outManifold.m_hasCollision = true;
        outManifold.m_contact.m_normal = GetNormalAt(sphereCenter.x, sphereCenter.z);
        outManifold.m_contact.m_penetration = groundHeight - sphereBottom;
        outManifold.m_contact.m_point = Vector3(sphereCenter.x, groundHeight, sphereCenter.z);

        return true;
    }

    bool HeightfieldColliderComponent::CheckAABBCollision(
        const Vector3& aabbMin,
        const Vector3& aabbMax,
        CollisionManifold& outManifold
    ) {
        // AABBの底面中心
        Vector3 center((aabbMin.x + aabbMax.x) * 0.5f, aabbMin.y, (aabbMin.z + aabbMax.z) * 0.5f);

        // 複数点でサンプリングして最大めり込みを見つける
        float maxPenetration = 0.0f;
        Vector3 maxNormal(0, 1, 0);
        Vector3 maxPoint = center;
        bool hasCollision = false;

        // AABBの4隅と中心をサンプリング
        float sampleX[] = { aabbMin.x, aabbMax.x, center.x };
        float sampleZ[] = { aabbMin.z, aabbMax.z, center.z };

        for (int ix = 0; ix < 3; ++ix) {
            for (int iz = 0; iz < 3; ++iz) {
                float x = sampleX[ix];
                float z = sampleZ[iz];
                float groundHeight = GetHeightAt(x, z);
                float penetration = groundHeight - aabbMin.y;

                if (penetration > 0.0f) {
                    hasCollision = true;
                    if (penetration > maxPenetration) {
                        maxPenetration = penetration;
                        maxNormal = GetNormalAt(x, z);
                        maxPoint = Vector3(x, groundHeight, z);
                    }
                }
            }
        }

        if (!hasCollision) {
            return false;
        }

        outManifold.m_hasCollision = true;
        outManifold.m_contact.m_normal = maxNormal;
        outManifold.m_contact.m_penetration = maxPenetration;
        outManifold.m_contact.m_point = maxPoint;

        return true;
    }

    bool HeightfieldColliderComponent::CheckCapsuleCollision(
        const Vector3& bottom,
        const Vector3& top,
        float radius,
        CollisionManifold& outManifold
    ) {
        // カプセルの下端を使用（地面との接触は主に下端）
        float groundHeight = GetHeightAt(bottom.x, bottom.z);
        float capsuleBottom = bottom.y - radius;

        if (capsuleBottom >= groundHeight) {
            return false;
        }

        outManifold.m_hasCollision = true;
        outManifold.m_contact.m_normal = GetNormalAt(bottom.x, bottom.z);
        outManifold.m_contact.m_penetration = groundHeight - capsuleBottom;
        outManifold.m_contact.m_point = Vector3(bottom.x, groundHeight, bottom.z);

        return true;
    }

    //============================================================
    // オーバーライド
    //============================================================

    bool HeightfieldColliderComponent::CheckCollision(ColliderComponent& other, CollisionManifold& outManifold) {
        outManifold.Reset();
        outManifold.m_colliderA = this;
        outManifold.m_colliderB = &other;

        bool result = false;

        switch (other.GetColliderType()) {
        case ColliderType::Sphere: {
            auto& sphere = static_cast<SphereColliderComponent&>(other);
            const auto& shape = sphere.GetWorldSphere();
            result = CheckSphereCollision(shape.m_center, shape.m_radius, outManifold);
            break;
        }
        case ColliderType::AABB: {
            auto& aabb = static_cast<AABBColliderComponent&>(other);
            const auto& shape = aabb.GetWorldAABB();
            result = CheckAABBCollision(shape.m_min, shape.m_max, outManifold);
            break;
        }
        case ColliderType::Capsule: {
            auto& capsule = static_cast<CapsuleColliderComponent&>(other);
            const auto& shape = capsule.GetWorldCapsule();
            result = CheckCapsuleCollision(
                shape.GetBottomSphereCenter(),
                shape.GetTopSphereCenter(),
                shape.m_radius,
                outManifold
            );
            break;
        }
        default:
            break;
        }

        return result;
    }

    //============================================================
    // Private
    //============================================================

    bool HeightfieldColliderComponent::IsInBounds(float x, float z) const {
        return x >= m_minX && x <= m_maxX && z >= m_minZ && z <= m_maxZ;
    }

} // namespace Engine
