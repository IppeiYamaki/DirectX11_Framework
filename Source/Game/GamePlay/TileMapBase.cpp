/// @file   TileMapBase.cpp
/// @brief  タイルマップ管理の基底クラス実装
#include "Game/Gameplay/TileMapBase.h"
#include "Game/Gameplay/RoomInstance.h"
#include "Game/Gameplay/TrapInstance.h"
#include "Game/Gameplay/CharacterInstance.h"
#include "Engine/Core/Logger.h"
#include <sstream>
#include <cmath>

namespace Game {

    TileMapBase::TileMapBase(int width, int height, float tileSize)
        : m_width(width)
        , m_height(height)
        , m_tileSize(tileSize)
        , m_mapOrigin(0.0f, 0.0f, 0.0f)
    {
        Initialize(width, height);
    }

    //============================================================
    // マップ初期化
    //============================================================

    void TileMapBase::Initialize(int width, int height) {
        m_width = width;
        m_height = height;
        m_tiles.clear();

        // 全タイルを生成
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                AddTile(GridPosition(x, y));
            }
        }
    }

    void TileMapBase::Clear() {
        m_tiles.clear();
    }

    //============================================================
    // マップ情報取得
    //============================================================

    int TileMapBase::GetWidth() const {
        return m_width;
    }

    int TileMapBase::GetHeight() const {
        return m_height;
    }

    float TileMapBase::GetTileSize() const {
        return m_tileSize;
    }

    void TileMapBase::SetTileSize(float size) {
        m_tileSize = size;
    }

    bool TileMapBase::IsValidPosition(const GridPosition& position) const {
        return position.x >= 0 && position.x < m_width &&
               position.y >= 0 && position.y < m_height;
    }

    //============================================================
    // タイル取得
    //============================================================

    TileCell* TileMapBase::GetTile(const GridPosition& position) {
        auto it = m_tiles.find(position);
        if (it != m_tiles.end()) {
            return &(it->second);
        }
        return nullptr;
    }

    const TileCell* TileMapBase::GetTile(const GridPosition& position) const {
        auto it = m_tiles.find(position);
        if (it != m_tiles.end()) {
            return &(it->second);
        }
        return nullptr;
    }

    TileCell* TileMapBase::GetTile(int x, int y) {
        return GetTile(GridPosition(x, y));
    }

    const TileCell* TileMapBase::GetTile(int x, int y) const {
        return GetTile(GridPosition(x, y));
    }

    //============================================================
    // 部屋管理
    //============================================================

    bool TileMapBase::PlaceRoom(const GridPosition& position, std::shared_ptr<RoomInstance> room) {
        TileCell* tile = GetTile(position);
        if (!tile) {
            return false;
        }
        if (!tile->CanPlaceRoom(room.get())) {
            return false;
        }
        tile->SetRoom(std::move(room));
        return true;
    }

    bool TileMapBase::RemoveRoom(const GridPosition& position) {
        TileCell* tile = GetTile(position);
        if (!tile) {
            return false;
        }
        return tile->RemoveRoom();
    }

    RoomInstance* TileMapBase::GetRoom(const GridPosition& position) const {
        const TileCell* tile = GetTile(position);
        if (!tile) {
            return nullptr;
        }
        return tile->GetRoom();
    }

    bool TileMapBase::MoveRoom(const GridPosition& from, const GridPosition& to) {
        TileCell* fromTile = GetTile(from);
        TileCell* toTile = GetTile(to);

        if (!fromTile || !toTile) {
            return false;
        }

        if (!fromTile->HasRoom()) {
            return false;
        }

        if (toTile->HasRoom()) {
            return false;
        }

        // 部屋のshared_ptrを取得して移動
        // 注意: TileCellから直接shared_ptrを取得するAPIが必要
        // 現在は部屋の移動機能は未実装
        Engine::Logger::Warn("MoveRoom: Room movement is not fully implemented yet");
        return false;
    }

    std::vector<GridPosition> TileMapBase::GetTilesWithRoomType(RoomType type) const {
        std::vector<GridPosition> result;
        for (const auto& [pos, tile] : m_tiles) {
            const RoomInstance* room = tile.GetRoom();
            if (room && room->GetRoomType() == type) {
                result.push_back(pos);
            }
        }
        return result;
    }

    std::vector<GridPosition> TileMapBase::GetTilesWithRooms() const {
        std::vector<GridPosition> result;
        for (const auto& [pos, tile] : m_tiles) {
            if (tile.HasRoom()) {
                result.push_back(pos);
            }
        }
        return result;
    }

    //============================================================
    // トラップ管理
    //============================================================

    bool TileMapBase::AddTrap(const GridPosition& position, std::shared_ptr<TrapInstance> trap) {
        TileCell* tile = GetTile(position);
        if (!tile) {
            return false;
        }
        tile->AddTrap(std::move(trap));
        return true;
    }

    bool TileMapBase::RemoveTrap(const GridPosition& position, TrapInstance* trap) {
        TileCell* tile = GetTile(position);
        if (!tile) {
            return false;
        }
        return tile->RemoveTrap(trap);
    }

    void TileMapBase::ClearTraps(const GridPosition& position) {
        TileCell* tile = GetTile(position);
        if (tile) {
            tile->ClearTraps();
        }
    }

    std::vector<GridPosition> TileMapBase::GetTilesWithTraps() const {
        std::vector<GridPosition> result;
        for (const auto& [pos, tile] : m_tiles) {
            if (tile.HasTraps()) {
                result.push_back(pos);
            }
        }
        return result;
    }

    //============================================================
    // キャラクター移動処理
    //============================================================

    bool TileMapBase::MoveCharacter(CharacterInstance* character,
                                    const GridPosition* from,
                                    const GridPosition& to) {
        if (!character) {
            return false;
        }

        TileCell* toTile = GetTile(to);
        if (!toTile) {
            return false;
        }

        if (!toTile->IsPassable()) {
            return false;
        }

        // 退室処理
        if (from) {
            ProcessCharacterExit(character, *from);
        }

        // 入室処理
        ProcessCharacterEnter(character, to);

        // キャラクターの位置を更新
        character->SetGridPosition(to);

        return true;
    }

    void TileMapBase::ProcessCharacterEnter(CharacterInstance* character, const GridPosition& position) {
        TileCell* tile = GetTile(position);
        if (tile) {
            tile->OnCharacterEnter(character);
        }
    }

    void TileMapBase::ProcessCharacterExit(CharacterInstance* character, const GridPosition& position) {
        TileCell* tile = GetTile(position);
        if (tile) {
            tile->OnCharacterExit(character);
        }
    }

    //============================================================
    // 座標変換
    //============================================================

    Engine::Vector3 TileMapBase::GridToWorld(const GridPosition& position) const {
        return Engine::Vector3(
            static_cast<float>(position.x) * m_tileSize + m_tileSize * 0.5f,
            0.0f,
            static_cast<float>(position.y) * m_tileSize + m_tileSize * 0.5f
        );
    }

    GridPosition TileMapBase::WorldToGrid(const Engine::Vector3& worldPos) const {
        int x = static_cast<int>(std::floor(worldPos.x / m_tileSize));
        int y = static_cast<int>(std::floor(worldPos.z / m_tileSize));
        return GridPosition(x, y);
    }

    Engine::Vector2 TileMapBase::GridToWorld2D(const GridPosition& position) const {
        return Engine::Vector2(
            static_cast<float>(position.x) * m_tileSize + m_tileSize * 0.5f,
            static_cast<float>(position.y) * m_tileSize + m_tileSize * 0.5f
        );
    }

    //============================================================
    // 隣接タイル取得
    //============================================================

    std::vector<GridPosition> TileMapBase::GetAdjacentPositions(const GridPosition& position) const {
        std::vector<GridPosition> result;
        
        // 4方向（上下左右）
        const GridPosition directions[] = {
            GridPosition(0, -1),  // 上
            GridPosition(0, 1),   // 下
            GridPosition(-1, 0),  // 左
            GridPosition(1, 0)    // 右
        };

        for (const auto& dir : directions) {
            GridPosition adjacent = position + dir;
            if (IsValidPosition(adjacent)) {
                result.push_back(adjacent);
            }
        }

        return result;
    }

    std::vector<GridPosition> TileMapBase::GetPassableAdjacentPositions(const GridPosition& position) const {
        std::vector<GridPosition> result;
        auto adjacents = GetAdjacentPositions(position);
        
        for (const auto& adj : adjacents) {
            const TileCell* tile = GetTile(adj);
            if (tile && tile->IsPassable()) {
                // タイルが通過可能かつ部屋がロックされていない場合のみ通過可能
                const RoomInstance* room = tile->GetRoom();
                if (!room || !room->IsLocked()) {
                    result.push_back(adj);
                }
            }
        }

        return result;
    }

    //============================================================
    // ターン処理
    //============================================================

    void TileMapBase::OnTurnEnd() {
        for (auto& [pos, tile] : m_tiles) {
            tile.OnTurnEnd();
        }
    }

    //============================================================
    // イテレーション
    //============================================================

    void TileMapBase::ForEachTile(const std::function<void(TileCell&)>& func) {
        for (auto& [pos, tile] : m_tiles) {
            func(tile);
        }
    }

    void TileMapBase::ForEachTile(const std::function<void(const TileCell&)>& func) const {
        for (const auto& [pos, tile] : m_tiles) {
            func(tile);
        }
    }

    //============================================================
    // デバッグ
    //============================================================

    void TileMapBase::DebugLog() const {
        std::ostringstream oss;
        oss << "TileMap: " << m_width << "x" << m_height << "\n";
        
        int roomCount = 0;
        int trapCount = 0;
        
        for (const auto& [pos, tile] : m_tiles) {
            if (tile.HasRoom()) {
                ++roomCount;
            }
            trapCount += static_cast<int>(tile.GetTrapCount());
        }
        
        oss << "Rooms: " << roomCount << ", Traps: " << trapCount;
        Engine::Logger::Info(oss.str());
    }

    //============================================================
    // Protected
    //============================================================

    void TileMapBase::AddTile(const GridPosition& position) {
        m_tiles.emplace(position, TileCell(position));
    }

    //============================================================
    // マップ原点管理
    //============================================================

    void TileMapBase::SetMapOrigin(const Engine::Vector3& origin) {
        m_mapOrigin = origin;
    }

    const Engine::Vector3& TileMapBase::GetMapOrigin() const {
        return m_mapOrigin;
    }

    Engine::Vector3 TileMapBase::GridToWorldPosition(const GridPosition& gridPos) const {
        return Engine::Vector3(
            m_mapOrigin.x + static_cast<float>(gridPos.x) * m_tileSize,
            m_mapOrigin.y,
            m_mapOrigin.z + static_cast<float>(gridPos.y) * m_tileSize
        );
    }

} // namespace Game
