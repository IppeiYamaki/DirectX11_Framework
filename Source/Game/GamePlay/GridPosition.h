/// @file   GridPosition.h
/// @brief  グリッド座標を表す構造体
#pragma once

#include <functional>

namespace Game {

    /// @brief グリッド座標を表す構造体
    /// @note  タイルマップ上の位置を整数座標で管理
    struct GridPosition {
        int x = 0;  ///< X座標
        int y = 0;  ///< Y座標

        /// @brief デフォルトコンストラクタ
        constexpr GridPosition() noexcept = default;

        /// @brief コンストラクタ
        /// @param x_ X座標
        /// @param y_ Y座標
        constexpr GridPosition(int x_, int y_) noexcept : x(x_), y(y_) {}

        /// @brief 等価比較演算子
        /// @param other 比較対象
        /// @return 等しければtrue
        constexpr bool operator==(const GridPosition& other) const noexcept {
            return x == other.x && y == other.y;
        }

        /// @brief 非等価比較演算子
        /// @param other 比較対象
        /// @return 等しくなければtrue
        constexpr bool operator!=(const GridPosition& other) const noexcept {
            return !(*this == other);
        }

        /// @brief 加算演算子
        /// @param other 加算する位置
        /// @return 加算結果
        constexpr GridPosition operator+(const GridPosition& other) const noexcept {
            return GridPosition(x + other.x, y + other.y);
        }

        /// @brief 減算演算子
        /// @param other 減算する位置
        /// @return 減算結果
        constexpr GridPosition operator-(const GridPosition& other) const noexcept {
            return GridPosition(x - other.x, y - other.y);
        }

        /// @brief マンハッタン距離を計算
        /// @param other 対象位置
        /// @return マンハッタン距離
        [[nodiscard]] constexpr int ManhattanDistance(const GridPosition& other) const noexcept {
            int dx = x - other.x;
            int dy = y - other.y;
            return (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
        }

        /// @brief 隣接しているか確認（4方向）
        /// @param other 対象位置
        /// @return 隣接していればtrue
        [[nodiscard]] constexpr bool IsAdjacent(const GridPosition& other) const noexcept {
            return ManhattanDistance(other) == 1;
        }
    };

} // namespace Game

namespace std {
    /// @brief GridPositionのハッシュ関数
    template<>
    struct hash<Game::GridPosition> {
        std::size_t operator()(const Game::GridPosition& pos) const noexcept {
            // より堅牢なハッシュ結合関数を使用
            // 大きな素数による乗算でハッシュ衝突を減らす
            return static_cast<std::size_t>(pos.x) * 73856093 ^ 
                   static_cast<std::size_t>(pos.y) * 19349663;
        }
    };
} // namespace std
