/// @file   TileCell.h
/// @brief  タイルセルクラス - 個々のタイルを管理
#pragma once

#include "Game/Gameplay/GridPosition.h"
#include <memory>
#include <vector>

namespace Game {

    class RoomInstance;
    class TrapInstance;
    class CharacterInstance;

    /// @brief タイルセルクラス
    /// @note  個々のタイルの状態（部屋、トラップ、キャラクター）を管理
    class TileCell {
    public:
        /// @brief コンストラクタ
        /// @param position グリッド上の位置
        explicit TileCell(const GridPosition& position = GridPosition());

        /// @brief デストラクタ
        ~TileCell() = default;

        // コピー禁止（moveは許可）
        TileCell(const TileCell&) = delete;
        TileCell& operator=(const TileCell&) = delete;
        TileCell(TileCell&&) noexcept = default;
        TileCell& operator=(TileCell&&) noexcept = default;

        //============================================================
        // 位置情報
        //============================================================

        /// @brief グリッド位置を取得
        /// @return グリッド位置
        [[nodiscard]] const GridPosition& GetPosition() const;

        /// @brief グリッド位置を設定
        /// @param position 設定する位置
        void SetPosition(const GridPosition& position);

        //============================================================
        // 部屋管理
        //============================================================

        /// @brief 部屋を設定
        /// @param room 設定する部屋（所有権を移動）
        void SetRoom(std::shared_ptr<RoomInstance> room);

        /// @brief 部屋を取得
        /// @return 部屋へのポインタ（未設定ならnullptr）
        [[nodiscard]] RoomInstance* GetRoom() const;

        /// @brief 部屋があるか確認
        /// @return 部屋があればtrue
        [[nodiscard]] bool HasRoom() const;

        /// @brief 部屋を削除
        /// @return 削除成功ならtrue（削除不可の部屋はfalse）
        bool RemoveRoom();

        /// @brief 部屋を配置可能か確認
        /// @param room 配置する部屋
        /// @return 配置可能ならtrue
        [[nodiscard]] bool CanPlaceRoom(const RoomInstance* room) const;

        //============================================================
        // トラップ管理（複数配置可能）
        //============================================================

        /// @brief トラップを追加
        /// @param trap 追加するトラップ（所有権を移動）
        void AddTrap(std::shared_ptr<TrapInstance> trap);

        /// @brief 全トラップを取得
        /// @return トラップのベクター
        [[nodiscard]] const std::vector<std::shared_ptr<TrapInstance>>& GetTraps() const;

        /// @brief トラップ数を取得
        /// @return トラップの数
        [[nodiscard]] std::size_t GetTrapCount() const;

        /// @brief トラップがあるか確認
        /// @return トラップがあればtrue
        [[nodiscard]] bool HasTraps() const;

        /// @brief 指定したトラップを削除
        /// @param trap 削除するトラップ
        /// @return 削除成功ならtrue
        bool RemoveTrap(TrapInstance* trap);

        /// @brief 全トラップを削除
        void ClearTraps();

        //============================================================
        // キャラクター入退室処理
        //============================================================

        /// @brief キャラクターがタイルに入った時の処理
        /// @param character 入ったキャラクター
        void OnCharacterEnter(CharacterInstance* character);

        /// @brief キャラクターがタイルから出た時の処理
        /// @param character 出たキャラクター
        void OnCharacterExit(CharacterInstance* character);

        //============================================================
        // 通過可能性
        //============================================================

        /// @brief 通過可能か確認
        /// @return 通過可能ならtrue
        [[nodiscard]] bool IsPassable() const;

        /// @brief 通過可能性を設定
        /// @param passable 通過可能ならtrue
        void SetPassable(bool passable);

        //============================================================
        // ターン処理
        //============================================================

        /// @brief ターン終了時の処理
        void OnTurnEnd();

    private:
        GridPosition m_position;                            ///< グリッド上の位置
        std::shared_ptr<RoomInstance> m_room;               ///< 配置されている部屋
        std::vector<std::shared_ptr<TrapInstance>> m_traps; ///< 配置されているトラップ
        bool m_isPassable = true;                           ///< 通過可能フラグ
    };

} // namespace Game
