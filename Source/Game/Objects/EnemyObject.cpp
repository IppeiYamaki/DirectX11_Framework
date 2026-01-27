/// @file   EnemyObject.cpp
/// @brief  敵キャラクターのGameObject実装

#include "EnemyObject.h"

#include "Engine/Scene/Components/Transform.h"
#include "Engine/Core/Logger.h"

#include <cfloat>

namespace Game {

    EnemyObject::EnemyObject(const std::string& name)
        : GameObject(name) {
        SetTag("Enemy");
    }

    //============================================================
    // Lifecycle
    //============================================================

    void EnemyObject::Initialize() {
        // 敵固有の初期化
        Engine::Logger::Info("EnemyObject initialized: " + GetName());
    }

    void EnemyObject::Update(float deltaTime) {
        if (IsDead()) return;

        // 振る舞いに基づく更新
        switch (m_behavior) {
        case EnemyBehavior::Idle:
            UpdateIdle(deltaTime);
            break;
        case EnemyBehavior::Patrol:
            UpdatePatrol(deltaTime);
            break;
        case EnemyBehavior::Chase:
            UpdateChase(deltaTime);
            break;
        case EnemyBehavior::Attack:
            UpdateAttack(deltaTime);
            break;
        }
    }

    void EnemyObject::Render() {
        // 敵固有の描画処理
        // コンポーネントベースの描画はDrawComponents()で自動的に行われる
    }

    void EnemyObject::OnDestroy() {
        Engine::Logger::Info("EnemyObject destroyed: " + GetName());
    }

    //============================================================
    // Enemy Specific
    //============================================================

    void EnemyObject::SetTarget(Engine::GameObject* target) {
        m_target = target;
    }

    Engine::GameObject* EnemyObject::GetTarget() const {
        return m_target;
    }

    void EnemyObject::SetBehavior(EnemyBehavior behavior) {
        if (m_behavior != behavior) {
            m_behavior = behavior;
            Engine::Logger::Info("Enemy behavior changed: " + GetName());
        }
    }

    EnemyBehavior EnemyObject::GetBehavior() const {
        return m_behavior;
    }

    void EnemyObject::SetMoveSpeed(float speed) {
        m_moveSpeed = speed;
    }

    float EnemyObject::GetMoveSpeed() const {
        return m_moveSpeed;
    }

    void EnemyObject::SetDetectionRange(float range) {
        m_detectionRange = range;
    }

    float EnemyObject::GetDetectionRange() const {
        return m_detectionRange;
    }

    void EnemyObject::SetAttackRange(float range) {
        m_attackRange = range;
    }

    float EnemyObject::GetAttackRange() const {
        return m_attackRange;
    }

    void EnemyObject::SetHP(int hp) {
        m_hp = hp;
        if (m_hp > m_maxHP) {
            m_hp = m_maxHP;
        }
        if (m_hp < 0) {
            m_hp = 0;
        }
    }

    int EnemyObject::GetHP() const {
        return m_hp;
    }

    void EnemyObject::TakeDamage(int damage) {
        m_hp -= damage;
        if (m_hp < 0) {
            m_hp = 0;
        }

        if (IsDead()) {
            Engine::Logger::Info("Enemy died: " + GetName());
        }
    }

    bool EnemyObject::IsDead() const {
        return m_hp <= 0;
    }

    //============================================================
    // Behavior Updates
    //============================================================

    void EnemyObject::UpdateIdle(float deltaTime) {
        (void)deltaTime;

        // ターゲットが検知範囲内にいれば追跡開始
        if (m_target && m_target->IsActive()) {
            float distance = GetDistanceToTarget();
            if (distance <= m_detectionRange) {
                SetBehavior(EnemyBehavior::Chase);
            }
        }
    }

    void EnemyObject::UpdatePatrol(float deltaTime) {
        (void)deltaTime;

        // 巡回ロジック（実装例）
        // 巡回ポイント間を移動する処理を記述

        // ターゲットが検知範囲内にいれば追跡開始
        if (m_target && m_target->IsActive()) {
            float distance = GetDistanceToTarget();
            if (distance <= m_detectionRange) {
                SetBehavior(EnemyBehavior::Chase);
            }
        }
    }

    void EnemyObject::UpdateChase(float deltaTime) {
        if (!m_target || !m_target->IsActive()) {
            SetBehavior(EnemyBehavior::Idle);
            return;
        }

        float distance = GetDistanceToTarget();

        // 攻撃範囲内なら攻撃
        if (distance <= m_attackRange) {
            SetBehavior(EnemyBehavior::Attack);
            return;
        }

        // 検知範囲外なら待機に戻る
        if (distance > m_detectionRange) {
            SetBehavior(EnemyBehavior::Idle);
            return;
        }

        // ターゲットに向かって移動
        Engine::Vector3 myPos = GetPosition();
        Engine::Vector3 targetPos = m_target->GetPosition();
        Engine::Vector3 direction = targetPos - myPos;
        
        if (direction.LengthSquared() > 0.0f) {
            direction.NormalizeInPlace();
            Engine::Vector3 newPos = myPos + direction * m_moveSpeed * deltaTime;
            SetPosition(newPos);
        }
    }

    void EnemyObject::UpdateAttack(float deltaTime) {
        (void)deltaTime;

        if (!m_target || !m_target->IsActive()) {
            SetBehavior(EnemyBehavior::Idle);
            return;
        }

        float distance = GetDistanceToTarget();

        // 攻撃範囲外なら追跡に戻る
        if (distance > m_attackRange) {
            SetBehavior(EnemyBehavior::Chase);
            return;
        }

        // 攻撃ロジック（実装例）
        // 攻撃アニメーションやダメージ処理を記述
    }

    float EnemyObject::GetDistanceToTarget() const {
        if (!m_target) return FLT_MAX;

        Engine::Vector3 myPos = GetPosition();
        Engine::Vector3 targetPos = m_target->GetPosition();
        Engine::Vector3 diff = targetPos - myPos;
        return diff.Length();
    }

} // namespace Game
