/// @file   ColliderComponent.h
/// @brief  コライダーコンポーネント基底クラス
#pragma once

#include "Engine/Scene/Component.h"
#include "Engine/Physics/ColliderTypes.h"
#include "Engine/Math/Vector3.h"

namespace Engine {

    class Transform;

    /// @brief コライダーコンポーネントの基底クラス
    /// @note  各形状（AABB, Sphere, Capsule等）はこのクラスを継承する
    class ColliderComponent : public Component {
    public:
        ColliderComponent();
        ~ColliderComponent() override = default;

        ColliderComponent(const ColliderComponent&) = delete;
        ColliderComponent& operator=(const ColliderComponent&) = delete;

        //============================================================
        // Lifecycle
        //============================================================

        /// @brief Awake時の初期化
        void OnAwake() override;

        /// @brief 毎フレーム更新
        /// @param deltaTime フレーム経過時間
        void Update(float deltaTime) override;

        //============================================================
        // 物理パラメータ
        //============================================================

        /// @brief 物理パラメータを設定
        /// @param desc 物理パラメータ
        void SetDesc(const ColliderDesc& desc);

        /// @brief 物理パラメータを取得
        /// @return 物理パラメータへの参照
        [[nodiscard]] const ColliderDesc& GetDesc() const;

        /// @brief 物理パラメータを取得（変更可能）
        /// @return 物理パラメータへの参照
        [[nodiscard]] ColliderDesc& GetDesc();

        /// @brief 質量を設定
        /// @param mass 質量（kg）
        void SetMass(float mass);

        /// @brief 質量を取得
        /// @return 質量
        [[nodiscard]] float GetMass() const;

        /// @brief 逆質量を取得（mass=0なら0を返す）
        /// @return 逆質量
        [[nodiscard]] float GetInverseMass() const;

        /// @brief 移動可能かを設定
        /// @param movable 移動可能フラグ
        void SetMovable(bool movable);

        /// @brief 移動可能かを取得
        /// @return 移動可能ならtrue
        [[nodiscard]] bool IsMovable() const;

        /// @brief 重力有効かを設定
        /// @param enable 重力有効フラグ
        void SetEnableGravity(bool enable);

        /// @brief 重力有効かを取得
        /// @return 重力有効ならtrue
        [[nodiscard]] bool IsGravityEnabled() const;

        //============================================================
        // 速度
        //============================================================

        /// @brief 線形速度を設定
        /// @param velocity 速度ベクトル
        void SetVelocity(const Vector3& velocity);

        /// @brief 線形速度を取得
        /// @return 速度ベクトル
        [[nodiscard]] const Vector3& GetVelocity() const;

        /// @brief 速度に加算
        /// @param delta 加算する速度
        void AddVelocity(const Vector3& delta);

        //============================================================
        // コライダー情報
        //============================================================

        /// @brief コライダータイプを取得
        /// @return コライダータイプ
        [[nodiscard]] ColliderType GetColliderType() const;

        /// @brief ワールド空間での形状を更新
        /// @note  派生クラスでオーバーライドする
        virtual void UpdateWorldShape();

        /// @brief 衝突検出
        /// @param other 相手コライダー
        /// @param outManifold 衝突結果出力
        /// @return 衝突していればtrue
        virtual bool CheckCollision(ColliderComponent& other, CollisionManifold& outManifold);

        //============================================================
        // Transform連携
        //============================================================

        /// @brief Transformを取得
        /// @return Transformへのポインタ
        [[nodiscard]] Transform* GetTransform();

        /// @brief Transformを取得（const版）
        /// @return Transformへのconstポインタ
        [[nodiscard]] const Transform* GetTransform() const;

        //============================================================
        // 接地判定
        //============================================================

        /// @brief 接地状態を設定
        /// @param grounded 接地しているか
        void SetGrounded(bool grounded);

        /// @brief 接地しているかを取得
        /// @return 接地していればtrue
        [[nodiscard]] bool IsGrounded() const;

        /// @brief 地面の法線を設定
        /// @param normal 地面法線
        void SetGroundNormal(const Vector3& normal);

        /// @brief 地面の法線を取得
        /// @return 地面法線
        [[nodiscard]] const Vector3& GetGroundNormal() const;

        //============================================================
        // コールバック（将来拡張用）
        //============================================================

        /// @brief 衝突時に呼ばれる（将来拡張用）
        /// @param info 衝突情報
        virtual void OnCollision(const CollisionInfo& info);

    protected:
        /// @brief コライダータイプを設定（派生クラスから呼ぶ）
        /// @param type コライダータイプ
        void SetColliderType(ColliderType type);

        /// @brief Transform参照をキャッシュ
        void CacheTransform();

    protected:
        ColliderType m_colliderType = ColliderType::None;   ///< コライダータイプ
        ColliderDesc m_desc;                                 ///< 物理パラメータ
        Vector3 m_velocity;                                  ///< 線形速度
        Transform* m_cachedTransform = nullptr;              ///< キャッシュされたTransform

        // 接地情報
        bool m_isGrounded = false;                           ///< 接地フラグ
        Vector3 m_groundNormal = Vector3(0, 1, 0);           ///< 地面法線
    };

} // namespace Engine
