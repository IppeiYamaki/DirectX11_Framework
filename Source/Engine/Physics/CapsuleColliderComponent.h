/// @file   CapsuleColliderComponent.h
/// @brief  カプセルコライダーコンポーネント
#pragma once

#include "Engine/Physics/ColliderComponent.h"

namespace Engine {

    /// @brief カプセルコライダーコンポーネント
    /// @note  プレイヤーキャラクターなど細長いオブジェクトに使用
    class CapsuleColliderComponent : public ColliderComponent {
    public:
        CapsuleColliderComponent();
        ~CapsuleColliderComponent() override = default;

        //============================================================
        // 形状設定
        //============================================================

        /// @brief 半径を設定
        /// @param radius 半球部分の半径
        void SetRadius(float radius);

        /// @brief 半径を取得
        /// @return 半径
        [[nodiscard]] float GetRadius() const;

        /// @brief 円筒部分の高さを設定
        /// @param height 円筒部分の高さ
        void SetHeight(float height);

        /// @brief 円筒部分の高さを取得
        /// @return 高さ
        [[nodiscard]] float GetHeight() const;

        /// @brief ローカルオフセットを設定
        /// @param offset Transformからのオフセット
        void SetOffset(const Vector3& offset);

        /// @brief ローカルオフセットを取得
        /// @return オフセット
        [[nodiscard]] const Vector3& GetOffset() const;

        /// @brief ワールド空間でのカプセルを取得
        /// @return CapsuleShape
        [[nodiscard]] const CapsuleShape& GetWorldCapsule() const;

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

        /// @brief Capsule vs Capsule
        bool CheckCapsulevsCapsule(CapsuleColliderComponent& other, CollisionManifold& outManifold);

        /// @brief Capsule vs Sphere
        bool CheckCapsulevsSphere(class SphereColliderComponent& other, CollisionManifold& outManifold);

        /// @brief Capsule vs AABB
        bool CheckCapsulevsAABB(class AABBColliderComponent& other, CollisionManifold& outManifold);

    private:
        float m_radius = 0.5f;          ///< ローカル半径
        float m_height = 1.0f;          ///< ローカル高さ（円筒部分）
        Vector3 m_offset;               ///< ローカルオフセット
        CapsuleShape m_worldShape;      ///< ワールド空間でのカプセル
    };

} // namespace Engine
