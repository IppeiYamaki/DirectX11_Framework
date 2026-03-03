/// @file   RoomInstance.h
/// @brief  部屋インスタンスの基底クラス
#pragma once

#include "Game/Gameplay/GridPosition.h"
#include <string>

namespace Game {

    class CharacterInstance;

    /// @brief 部屋の種類
    enum class RoomType {
        Corridor,       ///< 通路（効果なし）
        TrapRoom,       ///< 罠の部屋
        ResearchRoom,   ///< 研究資料室（ゲームの目標ポイント）
        EntryRoom,      ///< エントリー（敵の入口）
        Custom          ///< カスタム部屋
    };

    /// @brief 部屋インスタンスの基底クラス
    /// @note  各タイルに配置される部屋の基底クラス
    class RoomInstance {
    public:
        /// @brief コンストラクタ
        /// @param name 部屋名
        /// @param type 部屋の種類
        explicit RoomInstance(const std::string& name = "Room", RoomType type = RoomType::Corridor);

        /// @brief デストラクタ
        virtual ~RoomInstance() = default;

        // コピー禁止
        RoomInstance(const RoomInstance&) = delete;
        RoomInstance& operator=(const RoomInstance&) = delete;

        //============================================================
        // 識別情報
        //============================================================

        /// @brief 名前を設定
        /// @param name 部屋名
        void SetName(const std::string& name);

        /// @brief 名前を取得
        /// @return 部屋名
        [[nodiscard]] const std::string& GetName() const;

        /// @brief 部屋の種類を取得
        /// @return 部屋の種類
        [[nodiscard]] RoomType GetRoomType() const;

        //============================================================
        // 配置情報
        //============================================================

        /// @brief グリッド位置を設定
        /// @param position 設定する位置
        void SetGridPosition(const GridPosition& position);

        /// @brief グリッド位置を取得
        /// @return 現在のグリッド位置
        [[nodiscard]] const GridPosition& GetGridPosition() const;

        //============================================================
        // 状態管理
        //============================================================

        /// @brief 部屋がロックされているか確認
        /// @return ロックされていればtrue
        [[nodiscard]] bool IsLocked() const;

        /// @brief 部屋をロック
        /// @param turns ロックするターン数（デフォルト: 1）
        void Lock(int turns = 1);

        /// @brief 部屋のロックを解除
        void Unlock();

        /// @brief ターン終了時の処理（ロック解除など）
        virtual void OnTurnEnd();

        //============================================================
        // キャラクター効果のインターフェース
        //============================================================

        /// @brief キャラクターが部屋に入った時の効果
        /// @param character 入ったキャラクター
        virtual void OnCharacterEnter(CharacterInstance* character);

        /// @brief キャラクターが部屋から出た時の効果
        /// @param character 出たキャラクター
        virtual void OnCharacterExit(CharacterInstance* character);

        /// @brief 部屋の効果をキャラクターに適用
        /// @param character 対象キャラクター
        virtual void ApplyEffectToCharacter(CharacterInstance* character);

        //============================================================
        // 削除可能性
        //============================================================

        /// @brief 削除可能か確認
        /// @return 削除可能ならtrue
        [[nodiscard]] virtual bool CanBeRemoved() const;

    protected:
        std::string m_name;             ///< 部屋名
        RoomType m_type;                ///< 部屋の種類
        GridPosition m_gridPosition;   ///< グリッド上の位置

        bool m_isLocked = false;        ///< ロック状態
        int m_lockTurnsRemaining = 0;   ///< 残りロックターン数
    };

} // namespace Game
