/// @file   SphereColliderComponent.h
/// @brief  球コライダーコンポーネント
#pragma once

#include "Engine/Physics/ColliderComponent.h"

namespace Engine {

    /// @brief 球コライダーコンポーネント
    /// @note  球体オブジェクトに使用
    class SphereColliderComponent : public ColliderComponent {
    public:
        SphereColliderComponent();
        ~SphereColliderComponent() override = default;

        //============================================================
        // 形状設定
        //============================================================

        /// @brief 半径を設定
        /// @param radius 半径
        void SetRadius(float radius);

        /// @brief 半径を取得
        /// @return 半径
        [[nodiscard]] float GetRadius() const;

        /// @brief ローカルオフセットを設定
        /// @param offset Transformからのオフセット
        void SetOffset(const Vector3& offset);

        /// @brief ローカルオフセットを取得
        /// @return オフセット
        [[nodiscard]] const Vector3& GetOffset() const;

        /// @brief ワールド空間での球を取得
        /// @return SphereShape
        [[nodiscard]] const SphereShape& GetWorldSphere() const;

        //============================================================
        // オーバーライド
        //============================================================

        /// @brief ワールド空間での形状を更新
        void UpdateWorldShape() override;

        /// @brief 衝突検出
        /// @param other 相手コライダー
        /// @param outManifold 衝突結果出力
        /// @return 衝突していればtrue
        bool CheckCollision(ColliderComponent& other, CollisionManifold& outManifold) override;

    private:
        //============================================================
        // 内部衝突検出
        //============================================================

        /// @brief Sphere vs Sphere
        bool CheckSpherevsSphere(SphereColliderComponent& other, CollisionManifold& outManifold);

        /// @brief Sphere vs AABB（AABBCollider側で実装するのでダブルディスパッチ）
        bool CheckSpherevsAABB(class AABBColliderComponent& other, CollisionManifold& outManifold);

        /// @brief Sphere vs Capsule
        bool CheckSpherevsCapsule(class CapsuleColliderComponent& other, CollisionManifold& outManifold);

    private:
        float m_radius = 0.5f;          ///< ローカル半径
        Vector3 m_offset;               ///< ローカルオフセット
        SphereShape m_worldShape;       ///< ワールド空間での球
    };

} // namespace Engine
