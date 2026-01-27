/// @file   PlayerObject.cpp
/// @brief  プレイヤーキャラクターのGameObject実装

#include "PlayerObject.h"

#include "Engine/Scene/Components/Transform.h"
#include "Engine/Core/Logger.h"

namespace Game {

    PlayerObject::PlayerObject(const std::string& name)
        : GameObject(name) {
        SetTag("Player");
    }

    //============================================================
    // Lifecycle
    //============================================================

    void PlayerObject::Initialize() {
        // プレイヤー固有の初期化
        Engine::Logger::Info("PlayerObject initialized: " + GetName());
    }

    void PlayerObject::Update(float deltaTime) {
        if (IsDead()) return;

        // 入力処理
        HandleInput(deltaTime);
    }

    void PlayerObject::Render() {
        // プレイヤー固有の描画処理
        // コンポーネントベースの描画はDrawComponents()で自動的に行われる
    }

    //============================================================
    // Player Specific
    //============================================================

    void PlayerObject::SetMoveSpeed(float speed) {
        m_moveSpeed = speed;
    }

    float PlayerObject::GetMoveSpeed() const {
        return m_moveSpeed;
    }

    void PlayerObject::SetHP(int hp) {
        m_hp = hp;
        if (m_hp > m_maxHP) {
            m_hp = m_maxHP;
        }
        if (m_hp < 0) {
            m_hp = 0;
        }
    }

    int PlayerObject::GetHP() const {
        return m_hp;
    }

    void PlayerObject::TakeDamage(int damage) {
        m_hp -= damage;
        if (m_hp < 0) {
            m_hp = 0;
        }

        if (IsDead()) {
            Engine::Logger::Info("Player died: " + GetName());
        }
    }

    bool PlayerObject::IsDead() const {
        return m_hp <= 0;
    }

    void PlayerObject::HandleInput(float deltaTime) {
        // 入力に基づく移動処理のサンプル
        // 実際の入力処理は Input システムを使用して実装
        // ここではフレームワークの例として構造のみ示す

        Engine::Vector3 movement = Engine::Vector3::Zero();

        // 例: WASDキーによる移動
        // if (Input::IsKeyDown(KeyCode::W)) movement.z += 1.0f;
        // if (Input::IsKeyDown(KeyCode::S)) movement.z -= 1.0f;
        // if (Input::IsKeyDown(KeyCode::A)) movement.x -= 1.0f;
        // if (Input::IsKeyDown(KeyCode::D)) movement.x += 1.0f;

        if (movement.LengthSquared() > 0.0f) {
            movement.NormalizeInPlace();
            Engine::Vector3 newPos = GetPosition() + movement * m_moveSpeed * deltaTime;
            SetPosition(newPos);
        }
    }

} // namespace Game
