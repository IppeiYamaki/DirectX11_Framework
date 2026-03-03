/// @file   CharacterInstance.cpp
/// @brief  ゲーム内キャラクターの基底クラス実装
#include "Game/Gameplay/CharacterInstance.h"
#include "Game/Gameplay/RoomInstance.h"
#include "Game/Gameplay/TrapInstance.h"
#include "Game/Gameplay/TileCell.h"

namespace Game {

    CharacterInstance::CharacterInstance(const std::string& name)
        : m_name(name)
    {
    }

    //============================================================
    // 位置管理
    //============================================================

    void CharacterInstance::SetGridPosition(const GridPosition& position) {
        m_gridPosition = position;
    }

    const GridPosition& CharacterInstance::GetGridPosition() const {
        return m_gridPosition;
    }

    //============================================================
    // ステータス管理
    //============================================================

    void CharacterInstance::SetHealth(float hp) {
        m_health = hp;
        if (m_health > m_maxHealth) {
            m_health = m_maxHealth;
        }
        if (m_health < 0.0f) {
            m_health = 0.0f;
        }
    }

    float CharacterInstance::GetHealth() const {
        return m_health;
    }

    void CharacterInstance::SetMaxHealth(float maxHp) {
        m_maxHealth = maxHp;
        if (m_health > m_maxHealth) {
            m_health = m_maxHealth;
        }
    }

    float CharacterInstance::GetMaxHealth() const {
        return m_maxHealth;
    }

    void CharacterInstance::TakeDamage(float damage) {
        m_health -= damage;
        if (m_health < 0.0f) {
            m_health = 0.0f;
        }
    }

    bool CharacterInstance::IsAlive() const {
        return m_health > 0.0f;
    }

    void CharacterInstance::SetSanity(float sanity) {
        m_sanity = sanity;
        if (m_sanity > m_maxSanity) {
            m_sanity = m_maxSanity;
        }
        if (m_sanity < 0.0f) {
            m_sanity = 0.0f;
        }
    }

    float CharacterInstance::GetSanity() const {
        return m_sanity;
    }

    void CharacterInstance::SetMaxSanity(float maxSanity) {
        m_maxSanity = maxSanity;
        if (m_sanity > m_maxSanity) {
            m_sanity = m_maxSanity;
        }
    }

    float CharacterInstance::GetMaxSanity() const {
        return m_maxSanity;
    }

    void CharacterInstance::DecreaseSanity(float amount) {
        m_sanity -= amount;
        if (m_sanity < 0.0f) {
            m_sanity = 0.0f;
        }
    }

    bool CharacterInstance::IsInsane() const {
        return m_sanity <= 0.0f;
    }

    //============================================================
    // 識別情報
    //============================================================

    void CharacterInstance::SetName(const std::string& name) {
        m_name = name;
    }

    const std::string& CharacterInstance::GetName() const {
        return m_name;
    }

    //============================================================
    // 部屋・トラップ効果のインターフェース
    //============================================================

    void CharacterInstance::OnRoomEffect(RoomInstance* /*room*/) {
        // デフォルト実装は何もしない
        // 派生クラスで必要に応じてオーバーライド
    }

    void CharacterInstance::OnTrapEffect(TrapInstance* /*trap*/) {
        // デフォルト実装は何もしない
        // 派生クラスで必要に応じてオーバーライド
    }

    void CharacterInstance::OnEnterTile(TileCell* /*tile*/) {
        // デフォルト実装は何もしない
        // 派生クラスで必要に応じてオーバーライド
    }

    void CharacterInstance::OnExitTile(TileCell* /*tile*/) {
        // デフォルト実装は何もしない
        // 派生クラスで必要に応じてオーバーライド
    }

} // namespace Game
