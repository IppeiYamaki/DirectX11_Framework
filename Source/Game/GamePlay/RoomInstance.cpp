/// @file   RoomInstance.cpp
/// @brief  部屋インスタンスの基底クラス実装
#include "Game/Gameplay/RoomInstance.h"
#include "Game/Gameplay/CharacterInstance.h"

namespace Game {

    RoomInstance::RoomInstance(const std::string& name, RoomType type)
        : m_name(name)
        , m_type(type)
    {
    }

    //============================================================
    // 識別情報
    //============================================================

    void RoomInstance::SetName(const std::string& name) {
        m_name = name;
    }

    const std::string& RoomInstance::GetName() const {
        return m_name;
    }

    RoomType RoomInstance::GetRoomType() const {
        return m_type;
    }

    //============================================================
    // 配置情報
    //============================================================

    void RoomInstance::SetGridPosition(const GridPosition& position) {
        m_gridPosition = position;
    }

    const GridPosition& RoomInstance::GetGridPosition() const {
        return m_gridPosition;
    }

    //============================================================
    // 状態管理
    //============================================================

    bool RoomInstance::IsLocked() const {
        return m_isLocked;
    }

    void RoomInstance::Lock(int turns) {
        m_isLocked = true;
        m_lockTurnsRemaining = turns;
    }

    void RoomInstance::Unlock() {
        m_isLocked = false;
        m_lockTurnsRemaining = 0;
    }

    void RoomInstance::OnTurnEnd() {
        if (m_isLocked && m_lockTurnsRemaining > 0) {
            --m_lockTurnsRemaining;
            if (m_lockTurnsRemaining <= 0) {
                Unlock();
            }
        }
    }

    //============================================================
    // キャラクター効果のインターフェース
    //============================================================

    void RoomInstance::OnCharacterEnter(CharacterInstance* /*character*/) {
        // デフォルト実装は何もしない
        // 派生クラスで必要に応じてオーバーライド
    }

    void RoomInstance::OnCharacterExit(CharacterInstance* /*character*/) {
        // デフォルト実装は何もしない
        // 派生クラスで必要に応じてオーバーライド
    }

    void RoomInstance::ApplyEffectToCharacter(CharacterInstance* /*character*/) {
        // デフォルト実装は何もしない
        // 派生クラスで必要に応じてオーバーライド
    }

    //============================================================
    // 削除可能性
    //============================================================

    bool RoomInstance::CanBeRemoved() const {
        // 研究資料室は削除不可
        if (m_type == RoomType::ResearchRoom) {
            return false;
        }
        return true;
    }

} // namespace Game
