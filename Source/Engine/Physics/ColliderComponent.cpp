/// @file   ColliderComponent.cpp
/// @brief  コライダーコンポーネント基底クラス実装
#include "ColliderComponent.h"

#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Core/Logger.h"

namespace Engine {

    ColliderComponent::ColliderComponent()
        : m_colliderType(ColliderType::None)
        , m_desc()
        , m_velocity()
        , m_cachedTransform(nullptr)
        , m_isGrounded(false)
        , m_groundNormal(0, 1, 0) {
    }

    //============================================================
    // Lifecycle
    //============================================================

    void ColliderComponent::OnAwake() {
        CacheTransform();
    }

    void ColliderComponent::Update(float /*deltaTime*/) {
        // Transformが変更された場合に形状を更新
        UpdateWorldShape();
    }

    //============================================================
    // 物理パラメータ
    //============================================================

    void ColliderComponent::SetDesc(const ColliderDesc& desc) {
        m_desc = desc;
    }

    const ColliderDesc& ColliderComponent::GetDesc() const {
        return m_desc;
    }

    ColliderDesc& ColliderComponent::GetDesc() {
        return m_desc;
    }

    void ColliderComponent::SetMass(float mass) {
        m_desc.mass = mass;
    }

    float ColliderComponent::GetMass() const {
        return m_desc.mass;
    }

    float ColliderComponent::GetInverseMass() const {
        if (m_desc.mass <= 0.0f || !m_desc.isMovable) {
            return 0.0f;
        }
        return 1.0f / m_desc.mass;
    }

    void ColliderComponent::SetMovable(bool movable) {
        m_desc.isMovable = movable;
    }

    bool ColliderComponent::IsMovable() const {
        return m_desc.isMovable;
    }

    void ColliderComponent::SetEnableGravity(bool enable) {
        m_desc.enableGravity = enable;
    }

    bool ColliderComponent::IsGravityEnabled() const {
        return m_desc.enableGravity;
    }

    //============================================================
    // 速度
    //============================================================

    void ColliderComponent::SetVelocity(const Vector3& velocity) {
        m_velocity = velocity;
    }

    const Vector3& ColliderComponent::GetVelocity() const {
        return m_velocity;
    }

    void ColliderComponent::AddVelocity(const Vector3& delta) {
        m_velocity += delta;
    }

    //============================================================
    // コライダー情報
    //============================================================

    ColliderType ColliderComponent::GetColliderType() const {
        return m_colliderType;
    }

    void ColliderComponent::UpdateWorldShape() {
        // 基底クラスでは何もしない（派生クラスでオーバーライド）
    }

    bool ColliderComponent::CheckCollision(ColliderComponent& /*other*/, CollisionManifold& /*outManifold*/) {
        // 基底クラスでは衝突なし
        return false;
    }

    //============================================================
    // Transform連携
    //============================================================

    Transform* ColliderComponent::GetTransform() {
        if (!m_cachedTransform) {
            CacheTransform();
        }
        return m_cachedTransform;
    }

    const Transform* ColliderComponent::GetTransform() const {
        return m_cachedTransform;
    }

    //============================================================
    // 接地判定
    //============================================================

    void ColliderComponent::SetGrounded(bool grounded) {
        m_isGrounded = grounded;
    }

    bool ColliderComponent::IsGrounded() const {
        return m_isGrounded;
    }

    void ColliderComponent::SetGroundNormal(const Vector3& normal) {
        m_groundNormal = normal;
    }

    const Vector3& ColliderComponent::GetGroundNormal() const {
        return m_groundNormal;
    }

    //============================================================
    // コールバック
    //============================================================

    void ColliderComponent::OnCollision(const CollisionInfo& /*info*/) {
        // 基底クラスでは何もしない（派生クラスでオーバーライド）
    }

    //============================================================
    // Protected
    //============================================================

    void ColliderComponent::SetColliderType(ColliderType type) {
        m_colliderType = type;
    }

    void ColliderComponent::CacheTransform() {
        auto* owner = GetOwner();
        if (owner) {
            m_cachedTransform = owner->GetTransform();
        }
    }

} // namespace Engine
