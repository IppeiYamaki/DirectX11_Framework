/// @file   MazeGrid.h
/// @brief  迷路のグリッドデータを管理するクラス
#pragma once

#include <vector>
#include <cstdint>

namespace Game {

    /// @brief 迷路セルの種類
    enum class MazeCellType : uint8_t {
        Floor = 0,  ///< 床（歩ける）
        Wall = 1    ///< 壁
    };

    /// @brief 迷路設定パラメータ
    struct MazeSettings {
        float m_cellSize = 2.0f;         ///< 1セルのワールドサイズ
        float m_wallHeight = 3.0f;       ///< 壁の高さ
        int m_playerStartCellX = 1;      ///< プレイヤー開始セルX（グリッド座標）
        int m_playerStartCellZ = 1;      ///< プレイヤー開始セルZ（グリッド座標）
        float m_sphereScale = 0.4f;      ///< 球体のスケール
        float m_pickupRadius = 1.0f;     ///< 球体取得判定半径
        float m_playerRadius = 0.3f;     ///< プレイヤー衝突半径
        float m_playerEyeHeight = 1.9f;  ///< プレイヤー視点高さ
        float m_playerMoveSpeed = 2.0f;  ///< プレイヤー移動速度
        float m_mouseSensitivity = 0.05f; ///< マウス感度
    };

    /// @brief 迷路のグリッドデータを管理するクラス
    class MazeGrid {
    public:
        MazeGrid() = default;
        ~MazeGrid() = default;

        /// @brief 指定サイズで迷路を初期化
        /// @param width 幅（セル数）
        /// @param height 高さ（セル数）
        void Initialize(int width, int height);

        /// @brief 固定レイアウトの迷路をロード
        void LoadFixedLayout();

        /// @brief セルの種類を取得
        /// @param x X座標
        /// @param z Z座標
        /// @return セルの種類
        [[nodiscard]] MazeCellType GetCell(int x, int z) const;

        /// @brief セルの種類を設定
        /// @param x X座標
        /// @param z Z座標
        /// @param type セルの種類
        void SetCell(int x, int z, MazeCellType type);

        /// @brief グリッド座標からワールド座標へ変換
        /// @param gridX グリッドX
        /// @param gridZ グリッドZ
        /// @param cellSize セルサイズ
        /// @param outWorldX ワールドX出力
        /// @param outWorldZ ワールドZ出力
        static void GridToWorld(int gridX, int gridZ, float cellSize, float& outWorldX, float& outWorldZ);

        /// @brief ワールド座標からグリッド座標へ変換
        /// @param worldX ワールドX
        /// @param worldZ ワールドZ
        /// @param cellSize セルサイズ
        /// @param outGridX グリッドX出力
        /// @param outGridZ グリッドZ出力
        static void WorldToGrid(float worldX, float worldZ, float cellSize, int& outGridX, int& outGridZ);

        /// @brief 範囲内かチェック
        /// @param x X座標
        /// @param z Z座標
        /// @return 範囲内ならtrue
        [[nodiscard]] bool IsInBounds(int x, int z) const;

        /// @brief 歩行可能かチェック
        /// @param x X座標
        /// @param z Z座標
        /// @return 歩行可能ならtrue
        [[nodiscard]] bool IsWalkable(int x, int z) const;

        /// @brief 幅を取得
        [[nodiscard]] int GetWidth() const { return m_width; }

        /// @brief 高さを取得
        [[nodiscard]] int GetHeight() const { return m_height; }

        /// @brief 床セル数をカウント
        [[nodiscard]] int CountFloorCells() const;

    private:
        int m_width = 0;
        int m_height = 0;
        std::vector<MazeCellType> m_cells;
    };

} // namespace Game
