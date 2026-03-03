/// @file   TileCell.cpp
/// @brief  タイルセルクラス実装
#include "Game/Gameplay/TileCell.h"
#include "Game/Gameplay/RoomInstance.h"
#include "Game/Gameplay/TrapInstance.h"
#include "Game/Gameplay/CharacterInstance.h"
#include <algorithm>

namespace Game {

    TileCell::TileCell(const GridPosition& position)
        : m_position(position)
    {
    }

    //============================================================
    // 位置情報
    //============================================================

    const GridPosition& TileCell::GetPosition() const {
        return m_position;
    }

    void TileCell::SetPosition(const GridPosition& position) {
        m_position = position;
    }

    //============================================================
    // 部屋管理
    //============================================================

    void TileCell::SetRoom(std::shared_ptr<RoomInstance> room) {
        m_room = std::move(room);
        if (m_room) {
            m_room->SetGridPosition(m_position);
        }
    }

    RoomInstance* TileCell::GetRoom() const {
        return m_room.get();
    }

    bool TileCell::HasRoom() const {
        return m_room != nullptr;
    }

    bool TileCell::RemoveRoom() {
        if (!m_room) {
            return false;
        }
        if (!m_room->CanBeRemoved()) {
            return false;
        }
        m_room.reset();
        return true;
    }

    bool TileCell::CanPlaceRoom(const RoomInstance* /*room*/) const {
        // 既に部屋がある場合は配置不可
        return !HasRoom();
    }

    //============================================================
    // トラップ管理（複数配置可能）
    //============================================================

    void TileCell::AddTrap(std::shared_ptr<TrapInstance> trap) {
        if (trap) {
            trap->SetGridPosition(m_position);
            m_traps.push_back(std::move(trap));
        }
    }

    const std::vector<std::shared_ptr<TrapInstance>>& TileCell::GetTraps() const {
        return m_traps;
    }

    std::size_t TileCell::GetTrapCount() const {
        return m_traps.size();
    }

    bool TileCell::HasTraps() const {
        return !m_traps.empty();
    }

    bool TileCell::RemoveTrap(TrapInstance* trap) {
        auto it = std::find_if(m_traps.begin(), m_traps.end(),
            [trap](const std::shared_ptr<TrapInstance>& t) {
                return t.get() == trap;
            });
        if (it != m_traps.end()) {
            m_traps.erase(it);
            return true;
        }
        return false;
    }

    void TileCell::ClearTraps() {
        m_traps.clear();
    }

    //============================================================
    // キャラクター入退室処理
    //============================================================

    void TileCell::OnCharacterEnter(CharacterInstance* character) {
        if (!character) {
            return;
        }

        // キャラクターにタイル入室を通知
        character->OnEnterTile(this);

        // 部屋がロックされていない場合、部屋の効果を発動
        if (m_room && !m_room->IsLocked()) {
            m_room->OnCharacterEnter(character);
            character->OnRoomEffect(m_room.get());
        }

        // 全トラップを発動
        for (auto& trap : m_traps) {
            if (trap && trap->IsActive()) {
                trap->Trigger(character);
            }
        }

        // 使い果たしたトラップを削除
        m_traps.erase(
            std::remove_if(m_traps.begin(), m_traps.end(),
                [](const std::shared_ptr<TrapInstance>& t) {
                    return t->IsExhausted();
                }),
            m_traps.end()
        );
    }

    void TileCell::OnCharacterExit(CharacterInstance* character) {
        if (!character) {
            return;
        }

        // キャラクターにタイル退室を通知
        character->OnExitTile(this);

        // 部屋がある場合、退室処理
        if (m_room && !m_room->IsLocked()) {
            m_room->OnCharacterExit(character);
        }
    }

    //============================================================
    // 通過可能性
    //============================================================

    bool TileCell::IsPassable() const {
        return m_isPassable;
    }

    void TileCell::SetPassable(bool passable) {
        m_isPassable = passable;
    }

    //============================================================
    // ターン処理
    //============================================================

    void TileCell::OnTurnEnd() {
        // 部屋のターン終了処理
        if (m_room) {
            m_room->OnTurnEnd();
        }
    }

} // namespace Game
