/// @file   MazeGrid.cpp
/// @brief  迷路のグリッドデータを管理するクラス実装
#include "MazeGrid.h"

#include <cmath>

namespace Game {

    void MazeGrid::Initialize(int width, int height) {
        m_width = width;
        m_height = height;
        m_cells.resize(static_cast<size_t>(width * height), MazeCellType::Wall);
    }

    void MazeGrid::LoadFixedLayout() {
        // 15x15の固定迷路レイアウト
        // '#' = 壁, '.' = 床
        const char* layout[] = {
            "###############",
            "#.............#",
            "#.###.#####.#.#",
            "#.#...#...#.#.#",
            "#.#.###.#.#.#.#",
            "#.#.....#.#...#",
            "#.#######.###.#",
            "#.........#...#",
            "#.#######.#.###",
            "#.#.....#.#...#",
            "#.#.###.#.###.#",
            "#...#...#.....#",
            "#.###.#####.#.#",
            "#.............#",
            "###############"
        };

        const int layoutHeight = 15;
        const int layoutWidth = 15;

        Initialize(layoutWidth, layoutHeight);

        for (int z = 0; z < layoutHeight; ++z) {
            for (int x = 0; x < layoutWidth; ++x) {
                char c = layout[z][x];
                SetCell(x, z, (c == '#') ? MazeCellType::Wall : MazeCellType::Floor);
            }
        }
    }

    MazeCellType MazeGrid::GetCell(int x, int z) const {
        if (!IsInBounds(x, z)) {
            return MazeCellType::Wall;
        }
        return m_cells[static_cast<size_t>(z * m_width + x)];
    }

    void MazeGrid::SetCell(int x, int z, MazeCellType type) {
        if (!IsInBounds(x, z)) {
            return;
        }
        m_cells[static_cast<size_t>(z * m_width + x)] = type;
    }

    void MazeGrid::GridToWorld(int gridX, int gridZ, float cellSize, float& outWorldX, float& outWorldZ) {
        // セル中心のワールド座標
        outWorldX = (static_cast<float>(gridX) + 0.5f) * cellSize;
        outWorldZ = (static_cast<float>(gridZ) + 0.5f) * cellSize;
    }

    void MazeGrid::WorldToGrid(float worldX, float worldZ, float cellSize, int& outGridX, int& outGridZ) {
        outGridX = static_cast<int>(std::floor(worldX / cellSize));
        outGridZ = static_cast<int>(std::floor(worldZ / cellSize));
    }

    bool MazeGrid::IsInBounds(int x, int z) const {
        return x >= 0 && x < m_width && z >= 0 && z < m_height;
    }

    bool MazeGrid::IsWalkable(int x, int z) const {
        return IsInBounds(x, z) && GetCell(x, z) == MazeCellType::Floor;
    }

    int MazeGrid::CountFloorCells() const {
        int count = 0;
        for (const auto& cell : m_cells) {
            if (cell == MazeCellType::Floor) {
                ++count;
            }
        }
        return count;
    }

} // namespace Game
