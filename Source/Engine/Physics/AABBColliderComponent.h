/// @file   AABBColliderComponent.h
/// @brief  AABB（軸並行境界ボックス）コライダーコンポーネント
#pragma once

#include "Engine/Physics/ColliderComponent.h"

namespace Engine {

    /// @brief AABB（軸並行境界ボックス）コライダーコンポーネント
    /// @note  Cubeや箱型オブジェクトに使用
    class AABBColliderComponent : public ColliderComponent {
    public:
        AABBColliderComponent();
        ~AABBColliderComponent() override = default;

        //============================================================
        // 形状設定
        //============================================================

        /// @brief 半径（各軸の半分のサイズ）を設定
        /// @param halfExtents 半径
        void SetHalfExtents(const Vector3& halfExtents);

        /// @brief 半径を取得
        /// @return 半径
        [[nodiscard]] const Vector3& GetHalfExtents() const;

        /// @brief ローカルオフセットを設定
        /// @param offset Transformからのオフセット
        void SetOffset(const Vector3& offset);

        /// @brief ローカルオフセットを取得
        /// @return オフセット
        [[nodiscard]] const Vector3& GetOffset() const;

        /// @brief ワールド空間でのAABBを取得
        /// @return AABBShape
        [[nodiscard]] const AABBShape& GetWorldAABB() const;

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

        /// @brief AABB vs AABB
        bool CheckAABBvsAABB(AABBColliderComponent& other, CollisionManifold& outManifold);

        /// @brief AABB vs Sphere
        bool CheckAABBvsSphere(class SphereColliderComponent& other, CollisionManifold& outManifold);

        /// @brief AABB vs Capsule
        bool CheckAABBvsCapsule(class CapsuleColliderComponent& other, CollisionManifold& outManifold);

    private:
        Vector3 m_halfExtents = Vector3(0.5f, 0.5f, 0.5f);  ///< ローカル半径
        Vector3 m_offset;                                    ///< ローカルオフセット
        AABBShape m_worldShape;                              ///< ワールド空間でのAABB
    };

} // namespace Engine
