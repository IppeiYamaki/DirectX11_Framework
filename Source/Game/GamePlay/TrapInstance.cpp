/// @file   TrapInstance.cpp
/// @brief  トラップインスタンスの基底クラス実装
#include "Game/Gameplay/TrapInstance.h"
#include "Game/Gameplay/CharacterInstance.h"

namespace Game {

    TrapInstance::TrapInstance(const std::string& name, TrapType type)
        : m_name(name)
        , m_type(type)
    {
    }

    //============================================================
    // 識別情報
    //============================================================

    void TrapInstance::SetName(const std::string& name) {
        m_name = name;
    }

    const std::string& TrapInstance::GetName() const {
        return m_name;
    }

    TrapType TrapInstance::GetTrapType() const {
        return m_type;
    }

    //============================================================
    // 配置情報
    //============================================================

    void TrapInstance::SetGridPosition(const GridPosition& position) {
        m_gridPosition = position;
    }

    const GridPosition& TrapInstance::GetGridPosition() const {
        return m_gridPosition;
    }

    //============================================================
    // 状態管理
    //============================================================

    bool TrapInstance::IsActive() const {
        return m_isActive;
    }

    void TrapInstance::Activate() {
        m_isActive = true;
    }

    void TrapInstance::Deactivate() {
        m_isActive = false;
    }

    void TrapInstance::SetMaxUses(int uses) {
        m_maxUses = uses;
        m_remainingUses = uses;
    }

    int TrapInstance::GetRemainingUses() const {
        return m_remainingUses;
    }

    bool TrapInstance::IsExhausted() const {
        return m_maxUses >= 0 && m_remainingUses <= 0;
    }

    //============================================================
    // キャラクター効果のインターフェース
    //============================================================

    bool TrapInstance::Trigger(CharacterInstance* character) {
        // 非アクティブまたは使い果たした場合は発動しない
        if (!m_isActive || IsExhausted()) {
            return false;
        }

        // 効果を適用
        ApplyEffectToCharacter(character);

        // 使用回数を減少
        if (m_maxUses >= 0) {
            --m_remainingUses;
            if (m_remainingUses <= 0) {
                Deactivate();
            }
        }

        return true;
    }

    void TrapInstance::ApplyEffectToCharacter(CharacterInstance* character) {
        if (!character) {
            return;
        }

        // トラップの種類に応じた効果を適用
        switch (m_type) {
        case TrapType::Damage:
            character->TakeDamage(m_damage);
            break;
        case TrapType::SanityDrain:
            character->DecreaseSanity(m_sanityDrain);
            break;
        case TrapType::Slow:
            // 移動速度低下は派生クラスで実装
            break;
        case TrapType::Stun:
            // スタン効果は派生クラスで実装
            break;
        case TrapType::Custom:
            // カスタム効果は派生クラスで実装
            break;
        }

        // キャラクターにトラップ効果を通知
        character->OnTrapEffect(this);
    }

    //============================================================
    // 効果パラメータ
    //============================================================

    void TrapInstance::SetDamage(float damage) {
        m_damage = damage;
    }

    float TrapInstance::GetDamage() const {
        return m_damage;
    }

    void TrapInstance::SetSanityDrain(float sanityDrain) {
        m_sanityDrain = sanityDrain;
    }

    float TrapInstance::GetSanityDrain() const {
        return m_sanityDrain;
    }

    void TrapInstance::SetStunTurns(int turns) {
        m_stunTurns = turns;
    }

    int TrapInstance::GetStunTurns() const {
        return m_stunTurns;
    }

} // namespace Game
