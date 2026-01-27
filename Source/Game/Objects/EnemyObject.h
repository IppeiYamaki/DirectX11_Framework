/// @file   EnemyObject.h
/// @brief  敵キャラクターのGameObject
#pragma once

#include "Engine/Scene/GameObject.h"

namespace Game {

    /// @brief 敵の振る舞いタイプ
    enum class EnemyBehavior {
        Idle,       ///< 待機
        Patrol,     ///< 巡回
        Chase,      ///< 追跡
        Attack      ///< 攻撃
    };

    /// @brief 敵キャラクターのGameObject
    /// @note  敵固有の振る舞いを自己完結的に記述
    class EnemyObject : public Engine::GameObject {
    public:
        /// @brief コンストラクタ
        /// @param name オブジェクト名
        explicit EnemyObject(const std::string& name = "Enemy");

        /// @brief デストラクタ
        ~EnemyObject() override = default;

        //============================================================
        // Lifecycle
        //============================================================

        /// @brief 初期化処理
        void Initialize() override;

        /// @brief 毎フレームの処理
        /// @param deltaTime フレーム経過時間（秒）
        void Update(float deltaTime) override;

        /// @brief 描画処理
        void Render() override;

        /// @brief 破棄時の処理
        void OnDestroy() override;

        //============================================================
        // Enemy Specific
        //============================================================

        /// @brief ターゲットを設定
        /// @param target ターゲットGameObject
        void SetTarget(Engine::GameObject* target);

        /// @brief ターゲットを取得
        /// @return ターゲットGameObject
        [[nodiscard]] Engine::GameObject* GetTarget() const;

        /// @brief 振る舞いを設定
        /// @param behavior 振る舞いタイプ
        void SetBehavior(EnemyBehavior behavior);

        /// @brief 振る舞いを取得
        /// @return 振る舞いタイプ
        [[nodiscard]] EnemyBehavior GetBehavior() const;

        /// @brief 移動速度を設定
        /// @param speed 移動速度
        void SetMoveSpeed(float speed);

        /// @brief 移動速度を取得
        /// @return 移動速度
        [[nodiscard]] float GetMoveSpeed() const;

        /// @brief 検知範囲を設定
        /// @param range 検知範囲
        void SetDetectionRange(float range);

        /// @brief 検知範囲を取得
        /// @return 検知範囲
        [[nodiscard]] float GetDetectionRange() const;

        /// @brief 攻撃範囲を設定
        /// @param range 攻撃範囲
        void SetAttackRange(float range);

        /// @brief 攻撃範囲を取得
        /// @return 攻撃範囲
        [[nodiscard]] float GetAttackRange() const;

        /// @brief HPを設定
        /// @param hp HP値
        void SetHP(int hp);

        /// @brief HPを取得
        /// @return HP値
        [[nodiscard]] int GetHP() const;

        /// @brief ダメージを受ける
        /// @param damage ダメージ量
        void TakeDamage(int damage);

        /// @brief 死亡しているか確認
        /// @return 死亡していればtrue
        [[nodiscard]] bool IsDead() const;

    private:
        /// @brief 待機行動
        /// @param deltaTime フレーム経過時間
        void UpdateIdle(float deltaTime);

        /// @brief 巡回行動
        /// @param deltaTime フレーム経過時間
        void UpdatePatrol(float deltaTime);

        /// @brief 追跡行動
        /// @param deltaTime フレーム経過時間
        void UpdateChase(float deltaTime);

        /// @brief 攻撃行動
        /// @param deltaTime フレーム経過時間
        void UpdateAttack(float deltaTime);

        /// @brief ターゲットまでの距離を計算
        /// @return ターゲットまでの距離
        [[nodiscard]] float GetDistanceToTarget() const;

    private:
        Engine::GameObject* m_target = nullptr;         ///< ターゲット
        EnemyBehavior m_behavior = EnemyBehavior::Idle; ///< 現在の振る舞い

        float m_moveSpeed = 3.0f;           ///< 移動速度
        float m_detectionRange = 10.0f;     ///< 検知範囲
        float m_attackRange = 2.0f;         ///< 攻撃範囲

        int m_hp = 50;                      ///< HP
        int m_maxHP = 50;                   ///< 最大HP
    };

} // namespace Game
