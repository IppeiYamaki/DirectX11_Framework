/// @file   PhysicsSystem.cpp
/// @brief  物理シミュレーションシステム実装
#include "PhysicsSystem.h"

#include "ColliderComponent.h"
#include "ColliderTypes.h"
#include "HeightfieldColliderComponent.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Core/Logger.h"

#include <algorithm>
#include <cmath>

namespace Engine {

    namespace {
        /// @brief 接地判定のための法線Y成分の閾値（約60度の傾斜まで接地と判定）
        constexpr float kGroundingNormalThreshold = 0.5f;
    }

    PhysicsSystem::PhysicsSystem()
        : m_isEnabled(true)
        , m_debugDrawEnabled(false)
        , m_gravity(0, -9.81f, 0)
        , m_maxVelocity(50.0f) {
    }

    //============================================================
    // Lifecycle
    //============================================================

    bool PhysicsSystem::Initialize() {
        m_colliders.clear();
        m_dynamicColliders.clear();
        m_staticColliders.clear();
        Logger::Info("PhysicsSystem initialized.");
        return true;
    }

    void PhysicsSystem::Finalize() {
        m_colliders.clear();
        m_dynamicColliders.clear();
        m_staticColliders.clear();
        Logger::Info("PhysicsSystem finalized.");
    }

    //============================================================
    // 更新
    //============================================================

    void PhysicsSystem::Update(Scene* scene, float deltaTime) {
        if (!m_isEnabled || !scene) return;

        // 1) 全コライダー収集
        CollectColliders(scene);

        if (m_colliders.empty()) return;

        // 2) 重力適用
        ApplyGravity(deltaTime);

        // 3) 速度積分（位置更新）
        IntegrateVelocity(deltaTime);

        // 4) 衝突検出と解決
        DetectAndResolveCollisions();

        // 5) 速度クランプ
        ClampVelocities();

        // 6) Transformへ同期
        SyncToTransforms();
    }

    //============================================================
    // 設定
    //============================================================

    void PhysicsSystem::SetGravity(const Vector3& gravity) {
        m_gravity = gravity;
    }

    const Vector3& PhysicsSystem::GetGravity() const {
        return m_gravity;
    }

    void PhysicsSystem::SetMaxVelocity(float maxVelocity) {
        m_maxVelocity = maxVelocity;
    }

    float PhysicsSystem::GetMaxVelocity() const {
        return m_maxVelocity;
    }

    void PhysicsSystem::SetEnabled(bool enabled) {
        m_isEnabled = enabled;
    }

    bool PhysicsSystem::IsEnabled() const {
        return m_isEnabled;
    }

    //============================================================
    // デバッグ
    //============================================================

    void PhysicsSystem::SetDebugDrawEnabled(bool enabled) {
        m_debugDrawEnabled = enabled;
    }

    bool PhysicsSystem::IsDebugDrawEnabled() const {
        return m_debugDrawEnabled;
    }

    //============================================================
    // 内部処理
    //============================================================

    void PhysicsSystem::CollectColliders(Scene* scene) {
        m_colliders.clear();
        m_dynamicColliders.clear();
        m_staticColliders.clear();

        // シーンからColliderComponentを持つGameObjectを検索
        auto objects = scene->FindObjectsWithComponent<ColliderComponent>();
        
        for (auto* obj : objects) {
            if (!obj || !obj->IsActive()) continue;

            auto* collider = obj->GetComponent<ColliderComponent>();
            if (!collider || !collider->IsEnabled()) continue;

            // ワールド形状を更新
            collider->UpdateWorldShape();

            m_colliders.push_back(collider);

            // 動的/静的に分類
            if (collider->IsMovable()) {
                m_dynamicColliders.push_back(collider);
            }
            else {
                m_staticColliders.push_back(collider);
            }
        }
    }

    void PhysicsSystem::ApplyGravity(float deltaTime) {
        for (auto* collider : m_dynamicColliders) {
            if (!collider->IsGravityEnabled()) continue;
            if (collider->GetDesc().isTrigger) continue;

            // 接地中は重力を適用しない（または減衰）
            if (!collider->IsGrounded()) {
                collider->AddVelocity(m_gravity * deltaTime);
            }
        }
    }

    void PhysicsSystem::IntegrateVelocity(float deltaTime) {
        for (auto* collider : m_dynamicColliders) {
            auto* transform = collider->GetTransform();
            if (!transform) continue;

            const Vector3& vel = collider->GetVelocity();
            if (vel.LengthSquared() < 0.00001f) continue;

            // 位置を更新
            Vector3 pos = transform->GetWorldPosition();
            pos += vel * deltaTime;
            transform->SetWorldPosition(pos);
        }
    }

    void PhysicsSystem::DetectAndResolveCollisions() {
        // 接地フラグをリセット
        for (auto* collider : m_dynamicColliders) {
            collider->SetGrounded(false);
        }

        // 動的 vs 静的
        for (auto* dynamic : m_dynamicColliders) {
            for (auto* staticCol : m_staticColliders) {
                CollisionManifold manifold;
                bool hasCollision = false;

                // Heightfield は特別処理
                if (staticCol->GetColliderType() == ColliderType::Heightfield) {
                    hasCollision = staticCol->CheckCollision(*dynamic, manifold);
                }
                else {
                    hasCollision = dynamic->CheckCollision(*staticCol, manifold);
                }

                if (hasCollision) {
                    HandleTerrainCollision(dynamic, manifold);
                }
            }
        }

        // 動的 vs 動的
        for (size_t i = 0; i < m_dynamicColliders.size(); ++i) {
            for (size_t j = i + 1; j < m_dynamicColliders.size(); ++j) {
                CollisionManifold manifold;
                if (m_dynamicColliders[i]->CheckCollision(*m_dynamicColliders[j], manifold)) {
                    ResolveCollision(manifold);
                }
            }
        }
    }

    void PhysicsSystem::ClampVelocities() {
        for (auto* collider : m_dynamicColliders) {
            Vector3 vel = collider->GetVelocity();
            float speed = vel.Length();
            if (speed > m_maxVelocity) {
                vel = vel * (m_maxVelocity / speed);
                collider->SetVelocity(vel);
            }
        }
    }

    void PhysicsSystem::SyncToTransforms() {
        // IntegrateVelocityで既にTransformは更新されているため、
        // ここでは追加の同期処理が必要な場合のみ実装
    }

    void PhysicsSystem::ResolveCollision(CollisionManifold& manifold) {
        if (!manifold.m_hasCollision) return;

        auto* colliderA = manifold.m_colliderA;
        auto* colliderB = manifold.m_colliderB;

        if (!colliderA || !colliderB) return;

        // トリガーの場合は押し戻しなし
        if (colliderA->GetDesc().isTrigger || colliderB->GetDesc().isTrigger) {
            // コールバックのみ
            CollisionInfo infoA;
            infoA.m_other = colliderB;
            infoA.m_contact = manifold.m_contact;
            colliderA->OnCollision(infoA);

            CollisionInfo infoB;
            infoB.m_other = colliderA;
            infoB.m_contact = manifold.m_contact;
            infoB.m_contact.m_normal = -manifold.m_contact.m_normal;
            colliderB->OnCollision(infoB);
            return;
        }

        const Vector3& normal = manifold.m_contact.m_normal;
        float penetration = manifold.m_contact.m_penetration;

        // 逆質量を取得
        float invMassA = colliderA->GetInverseMass();
        float invMassB = colliderB->GetInverseMass();
        float invMassSum = invMassA + invMassB;

        if (invMassSum <= 0.0f) return; // 両方とも固定

        // 押し戻し量を質量比で分配
        float ratioA = invMassA / invMassSum;
        float ratioB = invMassB / invMassSum;

        // 位置補正
        auto* transformA = colliderA->GetTransform();
        auto* transformB = colliderB->GetTransform();

        if (transformA && colliderA->IsMovable()) {
            Vector3 pos = transformA->GetWorldPosition();
            pos -= normal * penetration * ratioA;
            transformA->SetWorldPosition(pos);
        }

        if (transformB && colliderB->IsMovable()) {
            Vector3 pos = transformB->GetWorldPosition();
            pos += normal * penetration * ratioB;
            transformB->SetWorldPosition(pos);
        }

        // 速度補正
        Vector3 velA = colliderA->GetVelocity();
        Vector3 velB = colliderB->GetVelocity();
        Vector3 relVel = velA - velB;

        float velAlongNormal = Vector3::Dot(relVel, normal);

        // 離れようとしている場合は何もしない
        if (velAlongNormal > 0) return;

        // 反発係数（平均）
        float restitution = (colliderA->GetDesc().restitution + colliderB->GetDesc().restitution) * 0.5f;

        // インパルスの大きさ
        float j = -(1.0f + restitution) * velAlongNormal;
        j /= invMassSum;

        // インパルスを適用
        Vector3 impulse = normal * j;

        if (colliderA->IsMovable()) {
            colliderA->SetVelocity(velA + impulse * invMassA);
        }
        if (colliderB->IsMovable()) {
            colliderB->SetVelocity(velB - impulse * invMassB);
        }
    }

    void PhysicsSystem::HandleTerrainCollision(ColliderComponent* collider, CollisionManifold& manifold) {
        if (!manifold.m_hasCollision || !collider) return;

        auto* transform = collider->GetTransform();
        if (!transform) return;

        const Vector3& normal = manifold.m_contact.m_normal;
        float penetration = manifold.m_contact.m_penetration;

        // 位置補正（地面の上に押し出す）
        Vector3 pos = transform->GetWorldPosition();
        pos += normal * penetration;
        transform->SetWorldPosition(pos);

        // 速度補正
        Vector3 vel = collider->GetVelocity();
        float velNormal = Vector3::Dot(vel, normal);

        if (velNormal < 0) {
            // 地面への速度成分を除去
            vel -= normal * velNormal;

            // 摩擦適用（接線方向の速度を減衰）
            float friction = collider->GetDesc().friction;
            Vector3 tangentVel = vel - normal * Vector3::Dot(vel, normal);
            vel = tangentVel * (1.0f - friction * 0.5f); // 簡易摩擦

            collider->SetVelocity(vel);
        }

        // 接地判定（法線が上向きに近い場合）
        if (normal.y > kGroundingNormalThreshold) {
            collider->SetGrounded(true);
            collider->SetGroundNormal(normal);
        }
    }

} // namespace Engine
