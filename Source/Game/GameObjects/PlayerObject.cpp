/// @file   PlayerObject.cpp
/// @brief  プレイヤーキャラクターのGameObject派生クラスの実装
#include "PlayerObject.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/Components/Transform.h"

namespace Game {

    //============================================================
    // コンストラクタ
    //============================================================

    PlayerObject::PlayerObject(const std::string& name)
        : GameObject(name)
        , m_moveSpeed(5.0f)
        , m_health(100.0f)
        , m_maxHealth(100.0f) {
        // プレイヤータグを設定
        SetTag("Player");
    }

    //============================================================
    // ライフサイクル
    //============================================================

    void PlayerObject::Initialize() {
        // 基底クラスの初期化（Transformが追加される）
        GameObject::Initialize();

        Engine::Logger::Info("PlayerObject initialized: " + GetName());
    }

    void PlayerObject::Update(float deltaTime) {
        if (!IsActive()) return;

        // 基底クラスの更新（コンポーネント更新）
        GameObject::Update(deltaTime);

        // プレイヤー固有の入力処理
        HandleInput(deltaTime);
    }

    void PlayerObject::Render() {
        if (!IsActive()) return;

        // 基底クラスの描画（コンポーネント描画）
        GameObject::Render();
    }

    void PlayerObject::OnDestroy() {
        Engine::Logger::Info("PlayerObject destroyed: " + GetName());

        // 基底クラスの破棄処理
        GameObject::OnDestroy();
    }

    //============================================================
    // プレイヤー固有機能
    //============================================================

    void PlayerObject::SetMoveSpeed(float speed) {
        m_moveSpeed = speed;
    }

    float PlayerObject::GetMoveSpeed() const {
        return m_moveSpeed;
    }

    void PlayerObject::SetHealth(float hp) {
        m_health = hp;
        if (m_health > m_maxHealth) {
            m_health = m_maxHealth;
        }
        if (m_health < 0.0f) {
            m_health = 0.0f;
        }
    }

    float PlayerObject::GetHealth() const {
        return m_health;
    }

    void PlayerObject::TakeDamage(float damage) {
        m_health -= damage;
        if (m_health <= 0.0f) {
            m_health = 0.0f;
            Engine::Logger::Info("Player defeated: " + GetName());
        }
    }

    bool PlayerObject::IsAlive() const {
        return m_health > 0.0f;
    }

    void PlayerObject::HandleInput(float deltaTime) {
        // 入力処理の例（実際の入力システムと連携する場合はここを拡張）
        // この例では、入力システムが別途実装されている想定
        (void)deltaTime;
    }

    void PlayerObject::Move(const Engine::Vector3& direction, float deltaTime) {
        if (auto* transform = GetTransform()) {
            Engine::Vector3 currentPos = transform->GetPosition();
            Engine::Vector3 movement = direction * m_moveSpeed * deltaTime;
            transform->SetPosition(currentPos + movement);
        }
    }

} // namespace Game
