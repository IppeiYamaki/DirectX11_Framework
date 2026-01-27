/// @file   EnemyObject.h
/// @brief  敵キャラクターのGameObject派生クラス
#pragma once

#include "Engine/Scene/GameObject.h"
#include "Engine/Math/Vector3.h"

namespace Game {

    /// @brief 敵キャラクター用のGameObject派生クラス
    /// @note  敵固有の挙動やAIロジックをこのクラス内で自己完結的に記述
    class EnemyObject : public Engine::GameObject {
    public:
        /// @brief 敵の状態
        enum class State {
            Idle,       ///< 待機中
            Patrol,     ///< 巡回中
            Chase,      ///< 追跡中
            Attack,     ///< 攻撃中
            Dead        ///< 死亡
        };

        /// @brief コンストラクタ
        /// @param name オブジェクト名（デフォルト: "Enemy"）
        explicit EnemyObject(const std::string& name = "Enemy");

        /// @brief デストラクタ
        ~EnemyObject() override = default;

        //============================================================
        // ライフサイクルオーバーライド
        //============================================================

        /// @brief 初期化処理
        void Initialize() override;

        /// @brief 毎フレームの更新処理
        /// @param deltaTime 前フレームからの経過時間（秒）
        void Update(float deltaTime) override;

        /// @brief 描画処理
        void Render() override;

        /// @brief 破棄処理
        void OnDestroy() override;

        //============================================================
        // 敵固有機能
        //============================================================

        /// @brief 移動速度を設定
        /// @param speed 移動速度
        void SetMoveSpeed(float speed);

        /// @brief 移動速度を取得
        /// @return 現在の移動速度
        [[nodiscard]] float GetMoveSpeed() const;

        /// @brief HPを設定
        /// @param hp HP値
        void SetHealth(float hp);

        /// @brief HPを取得
        /// @return 現在のHP
        [[nodiscard]] float GetHealth() const;

        /// @brief ダメージを受ける
        /// @param damage ダメージ量
        void TakeDamage(float damage);

        /// @brief 生存しているか確認
        /// @return 生存していればtrue
        [[nodiscard]] bool IsAlive() const;

        /// @brief 現在の状態を取得
        /// @return 現在の状態
        [[nodiscard]] State GetState() const;

        /// @brief 追跡対象を設定
        /// @param target 追跡対象のGameObject
        void SetTarget(Engine::GameObject* target);

        /// @brief 検知範囲を設定
        /// @param range 検知範囲
        void SetDetectionRange(float range);

        /// @brief 攻撃範囲を設定
        /// @param range 攻撃範囲
        void SetAttackRange(float range);

    protected:
        /// @brief AIの更新処理
        /// @param deltaTime フレーム経過時間
        virtual void UpdateAI(float deltaTime);

        /// @brief 状態を設定
        /// @param newState 新しい状態
        void SetState(State newState);

        /// @brief 待機処理
        /// @param deltaTime フレーム経過時間
        virtual void OnIdle(float deltaTime);

        /// @brief 巡回処理
        /// @param deltaTime フレーム経過時間
        virtual void OnPatrol(float deltaTime);

        /// @brief 追跡処理
        /// @param deltaTime フレーム経過時間
        virtual void OnChase(float deltaTime);

        /// @brief 攻撃処理
        /// @param deltaTime フレーム経過時間
        virtual void OnAttack(float deltaTime);

        /// @brief 死亡処理
        /// @param deltaTime フレーム経過時間
        virtual void OnDead(float deltaTime);

        /// @brief 移動処理
        /// @param direction 移動方向（正規化済み）
        /// @param deltaTime フレーム経過時間
        void Move(const Engine::Vector3& direction, float deltaTime);

        /// @brief ターゲットまでの距離を計算
        /// @return ターゲットまでの距離（ターゲットがなければ最大値）
        [[nodiscard]] float GetDistanceToTarget() const;

    private:
        float m_moveSpeed = 3.0f;           ///< 移動速度
        float m_health = 50.0f;             ///< HP
        float m_maxHealth = 50.0f;          ///< 最大HP
        float m_detectionRange = 10.0f;     ///< 検知範囲
        float m_attackRange = 2.0f;         ///< 攻撃範囲
        State m_state = State::Idle;        ///< 現在の状態
        Engine::GameObject* m_target = nullptr; ///< 追跡対象（借用）
    };

} // namespace Game
