/// @file   EnemyObject.cpp
/// @brief  敵キャラクターのGameObject派生クラスの実装
#include "EnemyObject.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/Components/Transform.h"

#include <cmath>
#include <limits>

namespace Game {

    //============================================================
    // コンストラクタ
    //============================================================

    EnemyObject::EnemyObject(const std::string& name)
        : GameObject(name)
        , m_moveSpeed(3.0f)
        , m_health(50.0f)
        , m_maxHealth(50.0f)
        , m_detectionRange(10.0f)
        , m_attackRange(2.0f)
        , m_state(State::Idle)
        , m_target(nullptr) {
        // 敵タグを設定
        SetTag("Enemy");
    }

    //============================================================
    // ライフサイクル
    //============================================================

    void EnemyObject::Initialize() {
        // 基底クラスの初期化（Transformが追加される）
        GameObject::Initialize();

        Engine::Logger::Info("EnemyObject initialized: " + GetName());
    }

    void EnemyObject::Update(float deltaTime) {
        if (!IsActive()) return;

        // 基底クラスの更新（コンポーネント更新）
        GameObject::Update(deltaTime);

        // AI更新
        UpdateAI(deltaTime);
    }

    void EnemyObject::Render() {
        if (!IsActive()) return;

        // 基底クラスの描画（コンポーネント描画）
        GameObject::Render();
    }

    void EnemyObject::OnDestroy() {
        Engine::Logger::Info("EnemyObject destroyed: " + GetName());

        // 基底クラスの破棄処理
        GameObject::OnDestroy();
    }

    //============================================================
    // 敵固有機能
    //============================================================

    void EnemyObject::SetMoveSpeed(float speed) {
        m_moveSpeed = speed;
    }

    float EnemyObject::GetMoveSpeed() const {
        return m_moveSpeed;
    }

    void EnemyObject::SetHealth(float hp) {
        m_health = hp;
        if (m_health > m_maxHealth) {
            m_health = m_maxHealth;
        }
        if (m_health < 0.0f) {
            m_health = 0.0f;
        }
    }

    float EnemyObject::GetHealth() const {
        return m_health;
    }

    void EnemyObject::TakeDamage(float damage) {
        if (m_state == State::Dead) return;

        m_health -= damage;
        if (m_health <= 0.0f) {
            m_health = 0.0f;
            SetState(State::Dead);
            Engine::Logger::Info("Enemy defeated: " + GetName());
        }
    }

    bool EnemyObject::IsAlive() const {
        return m_health > 0.0f;
    }

    EnemyObject::State EnemyObject::GetState() const {
        return m_state;
    }

    void EnemyObject::SetTarget(Engine::GameObject* target) {
        m_target = target;
    }

    void EnemyObject::SetDetectionRange(float range) {
        m_detectionRange = range;
    }

    void EnemyObject::SetAttackRange(float range) {
        m_attackRange = range;
    }

    //============================================================
    // AI処理
    //============================================================

    void EnemyObject::UpdateAI(float deltaTime) {
        switch (m_state) {
        case State::Idle:
            OnIdle(deltaTime);
            break;
        case State::Patrol:
            OnPatrol(deltaTime);
            break;
        case State::Chase:
            OnChase(deltaTime);
            break;
        case State::Attack:
            OnAttack(deltaTime);
            break;
        case State::Dead:
            OnDead(deltaTime);
            break;
        }
    }

    void EnemyObject::SetState(State newState) {
        if (m_state != newState) {
            m_state = newState;
        }
    }

    void EnemyObject::OnIdle(float deltaTime) {
        (void)deltaTime;

        // ターゲットが検知範囲内にいれば追跡開始
        float distance = GetDistanceToTarget();
        if (distance < m_detectionRange) {
            SetState(State::Chase);
        }
    }

    void EnemyObject::OnPatrol(float deltaTime) {
        (void)deltaTime;

        // 巡回ロジック（派生クラスで実装可能）
        float distance = GetDistanceToTarget();
        if (distance < m_detectionRange) {
            SetState(State::Chase);
        }
    }

    void EnemyObject::OnChase(float deltaTime) {
        if (!m_target || !m_target->IsActive()) {
            SetState(State::Idle);
            return;
        }

        float distance = GetDistanceToTarget();

        // 攻撃範囲内なら攻撃
        if (distance < m_attackRange) {
            SetState(State::Attack);
            return;
        }

        // 検知範囲外なら待機に戻る
        if (distance > m_detectionRange * 1.5f) {
            SetState(State::Idle);
            return;
        }

        // ターゲットに向かって移動
        Engine::Vector3 myPos = GetPosition();
        Engine::Vector3 targetPos = m_target->GetPosition();
        Engine::Vector3 direction = targetPos - myPos;
        
        float length = direction.Length();
        if (length > 0.001f) {
            direction = direction / length; // 正規化
            Move(direction, deltaTime);
        }
    }

    void EnemyObject::OnAttack(float deltaTime) {
        (void)deltaTime;

        // 攻撃ロジック（派生クラスで実装可能）
        float distance = GetDistanceToTarget();
        if (distance > m_attackRange) {
            SetState(State::Chase);
        }
    }

    void EnemyObject::OnDead(float deltaTime) {
        (void)deltaTime;

        // 死亡時の処理（アニメーション再生、破棄予約など）
    }

    void EnemyObject::Move(const Engine::Vector3& direction, float deltaTime) {
        if (auto* transform = GetTransform()) {
            Engine::Vector3 currentPos = transform->GetPosition();
            Engine::Vector3 movement = direction * m_moveSpeed * deltaTime;
            transform->SetPosition(currentPos + movement);
        }
    }

    float EnemyObject::GetDistanceToTarget() const {
        if (!m_target) {
            return std::numeric_limits<float>::max();
        }

        Engine::Vector3 myPos = GetPosition();
        Engine::Vector3 targetPos = m_target->GetPosition();
        Engine::Vector3 diff = targetPos - myPos;
        return diff.Length();
    }

} // namespace Game
