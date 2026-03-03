/// @file   PhysicsSystem.h
/// @brief  物理シミュレーションシステム
#pragma once

#include "Engine/Math/Vector3.h"
#include <vector>

namespace Engine {

    class Scene;
    class ColliderComponent;
    struct CollisionManifold;

    /// @brief 物理シミュレーションシステム
    /// @note  毎フレームUpdate()を呼び出して物理演算を実行する
    class PhysicsSystem final {
    public:
        PhysicsSystem();
        ~PhysicsSystem() = default;

        PhysicsSystem(const PhysicsSystem&) = delete;
        PhysicsSystem& operator=(const PhysicsSystem&) = delete;

        //============================================================
        // Lifecycle
        //============================================================

        /// @brief 初期化
        /// @return 成功ならtrue
        [[nodiscard]] bool Initialize();

        /// @brief 終了処理
        void Finalize();

        //============================================================
        // 更新
        //============================================================

        /// @brief 物理更新
        /// @param scene シーン
        /// @param deltaTime フレーム経過時間
        void Update(Scene* scene, float deltaTime);

        //============================================================
        // 設定
        //============================================================

        /// @brief 重力を設定
        /// @param gravity 重力ベクトル
        void SetGravity(const Vector3& gravity);

        /// @brief 重力を取得
        /// @return 重力ベクトル
        [[nodiscard]] const Vector3& GetGravity() const;

        /// @brief 最大速度を設定
        /// @param maxVelocity 最大速度
        void SetMaxVelocity(float maxVelocity);

        /// @brief 最大速度を取得
        /// @return 最大速度
        [[nodiscard]] float GetMaxVelocity() const;

        /// @brief 有効/無効を設定
        /// @param enabled 有効フラグ
        void SetEnabled(bool enabled);

        /// @brief 有効かどうかを取得
        /// @return 有効ならtrue
        [[nodiscard]] bool IsEnabled() const;

        //============================================================
        // デバッグ
        //============================================================

        /// @brief デバッグ描画を有効化
        /// @param enabled 有効フラグ
        void SetDebugDrawEnabled(bool enabled);

        /// @brief デバッグ描画が有効かどうか
        /// @return 有効ならtrue
        [[nodiscard]] bool IsDebugDrawEnabled() const;

    private:
        //============================================================
        // 内部処理
        //============================================================

        /// @brief シーンからすべてのコライダーを収集
        /// @param scene シーン
        void CollectColliders(Scene* scene);

        /// @brief 重力を適用
        /// @param deltaTime フレーム経過時間
        void ApplyGravity(float deltaTime);

        /// @brief 速度を積分して位置を更新（予測移動）
        /// @param deltaTime フレーム経過時間
        void IntegrateVelocity(float deltaTime);

        /// @brief 衝突検出と解決
        void DetectAndResolveCollisions();

        /// @brief 速度の上限をクランプ
        void ClampVelocities();

        /// @brief Transformへ結果を書き戻す
        void SyncToTransforms();

        /// @brief 衝突解決
        /// @param manifold 衝突マニフォールド
        void ResolveCollision(CollisionManifold& manifold);

        /// @brief 地形衝突処理
        /// @param collider 衝突対象コライダー
        /// @param manifold 衝突マニフォールド
        void HandleTerrainCollision(ColliderComponent* collider, CollisionManifold& manifold);

    private:
        bool m_isEnabled = true;                                ///< 有効フラグ
        bool m_debugDrawEnabled = false;                        ///< デバッグ描画フラグ

        Vector3 m_gravity = Vector3(0, -9.81f, 0);              ///< 重力ベクトル
        float m_maxVelocity = 50.0f;                            ///< 最大速度

        std::vector<ColliderComponent*> m_colliders;            ///< 収集されたコライダー
        std::vector<ColliderComponent*> m_dynamicColliders;     ///< 動的コライダー
        std::vector<ColliderComponent*> m_staticColliders;      ///< 静的コライダー（地形含む）
    };

} // namespace Engine
