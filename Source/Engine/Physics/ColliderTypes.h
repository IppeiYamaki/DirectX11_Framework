/// @file   ColliderTypes.h
/// @brief  コライダー関連の型定義と構造体
#pragma once

#include "Engine/Math/Vector3.h"
#include <cstdint>

namespace Engine {

    /// @brief コライダーの形状タイプ
    enum class ColliderType : std::uint8_t {
        None = 0,       ///< 無効
        AABB,           ///< 軸並行境界ボックス
        Sphere,         ///< 球
        Capsule,        ///< カプセル
        Heightfield,    ///< ハイトフィールド（地形用）
        Plane,          ///< 無限平面
        // 将来拡張用
        Mesh,           ///< メッシュコライダー（将来実装）
    };

    /// @brief コライダーの物理パラメータ
    struct ColliderDesc {
        float mass = 1.0f;              ///< 質量（kg）。0以下は無限質量扱い
        bool isMovable = true;          ///< 動的オブジェクトか（falseなら固定物）
        bool enableGravity = true;      ///< 重力を受けるか
        bool isTrigger = false;         ///< トリガーモード（衝突イベントのみ、押し戻ししない）
        float restitution = 0.0f;       ///< 反発係数（0-1）
        float friction = 0.5f;          ///< 摩擦係数（0-1）

        ColliderDesc() = default;

        /// @brief 静的オブジェクト用の設定を返す
        static ColliderDesc Static() {
            ColliderDesc desc;
            desc.mass = 0.0f;
            desc.isMovable = false;
            desc.enableGravity = false;
            return desc;
        }

        /// @brief 動的オブジェクト用の設定を返す
        /// @param mass 質量
        static ColliderDesc Dynamic(float mass = 1.0f) {
            ColliderDesc desc;
            desc.mass = mass;
            desc.isMovable = true;
            desc.enableGravity = true;
            return desc;
        }

        /// @brief キネマティックオブジェクト用の設定を返す（動くが物理影響を受けない）
        static ColliderDesc Kinematic() {
            ColliderDesc desc;
            desc.mass = 0.0f;
            desc.isMovable = true;
            desc.enableGravity = false;
            return desc;
        }
    };

    /// @brief AABB（軸並行境界ボックス）の形状データ
    struct AABBShape {
        Vector3 m_min;          ///< 最小座標
        Vector3 m_max;          ///< 最大座標
        Vector3 m_halfExtents;  ///< 半径（各軸）

        AABBShape() : m_min(), m_max(), m_halfExtents(0.5f, 0.5f, 0.5f) {}

        /// @brief 中心を取得
        [[nodiscard]] Vector3 GetCenter() const {
            return (m_min + m_max) * 0.5f;
        }

        /// @brief サイズを取得
        [[nodiscard]] Vector3 GetSize() const {
            return m_max - m_min;
        }

        /// @brief 半径から更新
        /// @param center 中心位置
        /// @param halfExtents 半径
        void SetFromCenterAndHalfExtents(const Vector3& center, const Vector3& halfExtents) {
            m_halfExtents = halfExtents;
            m_min = center - halfExtents;
            m_max = center + halfExtents;
        }
    };

    /// @brief 球の形状データ
    struct SphereShape {
        Vector3 m_center;   ///< 中心位置
        float m_radius;     ///< 半径

        SphereShape() : m_center(), m_radius(0.5f) {}
    };

    /// @brief カプセルの形状データ（Y軸方向）
    struct CapsuleShape {
        Vector3 m_center;   ///< カプセルの中心位置
        float m_radius;     ///< 半球部分の半径
        float m_height;     ///< 円筒部分の高さ（全体高さ = height + radius * 2）

        CapsuleShape() : m_center(), m_radius(0.5f), m_height(1.0f) {}

        /// @brief 上端の半球中心を取得
        [[nodiscard]] Vector3 GetTopSphereCenter() const {
            return m_center + Vector3(0, m_height * 0.5f, 0);
        }

        /// @brief 下端の半球中心を取得
        [[nodiscard]] Vector3 GetBottomSphereCenter() const {
            return m_center - Vector3(0, m_height * 0.5f, 0);
        }

        /// @brief 全体の高さを取得
        [[nodiscard]] float GetTotalHeight() const {
            return m_height + m_radius * 2.0f;
        }
    };

    /// @brief 衝突点情報
    struct ContactPoint {
        Vector3 m_point;            ///< 接触点（ワールド座標）
        Vector3 m_normal;           ///< 接触法線（A→Bの方向）
        float m_penetration;        ///< めり込み深度（正の値）

        ContactPoint()
            : m_point(), m_normal(0, 1, 0), m_penetration(0) {}
    };

    /// @brief 衝突マニフォールド（衝突検出結果）
    struct CollisionManifold {
        bool m_hasCollision = false;        ///< 衝突しているか
        ContactPoint m_contact;             ///< 代表接触点（簡易版では1点のみ）
        class ColliderComponent* m_colliderA = nullptr;  ///< コライダーA
        class ColliderComponent* m_colliderB = nullptr;  ///< コライダーB

        CollisionManifold() = default;

        /// @brief 衝突なし状態にリセット
        void Reset() {
            m_hasCollision = false;
            m_contact = ContactPoint();
            m_colliderA = nullptr;
            m_colliderB = nullptr;
        }
    };

    /// @brief 衝突イベント情報（コールバック用）
    struct CollisionInfo {
        class ColliderComponent* m_other = nullptr;  ///< 相手コライダー
        ContactPoint m_contact;                       ///< 接触点情報
        bool m_isEnter = false;                       ///< 衝突開始フレームか
        bool m_isExit = false;                        ///< 衝突終了フレームか
    };

} // namespace Engine
