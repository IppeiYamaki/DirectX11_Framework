/// @file   CharacterInstance.h
/// @brief  ゲーム内キャラクターの基底クラス
#pragma once

#include "Game/Gameplay/GridPosition.h"
#include <string>

namespace Game {

    class RoomInstance;
    class TrapInstance;
    class TileCell;

    /// @brief ゲーム内キャラクターの基底クラス
    /// @note  敵キャラクターやプレイヤーキャラクターの共通機能を定義
    class CharacterInstance {
    public:
        /// @brief コンストラクタ
        /// @param name キャラクター名
        explicit CharacterInstance(const std::string& name = "Character");

        /// @brief デストラクタ
        virtual ~CharacterInstance() = default;

        // コピー禁止
        CharacterInstance(const CharacterInstance&) = delete;
        CharacterInstance& operator=(const CharacterInstance&) = delete;

        //============================================================
        // 位置管理
        //============================================================

        /// @brief グリッド位置を設定
        /// @param position 設定する位置
        void SetGridPosition(const GridPosition& position);

        /// @brief グリッド位置を取得
        /// @return 現在のグリッド位置
        [[nodiscard]] const GridPosition& GetGridPosition() const;

        //============================================================
        // ステータス管理
        //============================================================

        /// @brief 体力を設定
        /// @param hp 体力値
        void SetHealth(float hp);

        /// @brief 体力を取得
        /// @return 現在の体力
        [[nodiscard]] float GetHealth() const;

        /// @brief 最大体力を設定
        /// @param maxHp 最大体力値
        void SetMaxHealth(float maxHp);

        /// @brief 最大体力を取得
        /// @return 最大体力
        [[nodiscard]] float GetMaxHealth() const;

        /// @brief ダメージを受ける
        /// @param damage ダメージ量
        void TakeDamage(float damage);

        /// @brief 生存しているか確認
        /// @return 生存していればtrue
        [[nodiscard]] bool IsAlive() const;

        /// @brief 正気度を設定
        /// @param sanity 正気度値
        void SetSanity(float sanity);

        /// @brief 正気度を取得
        /// @return 現在の正気度
        [[nodiscard]] float GetSanity() const;

        /// @brief 最大正気度を設定
        /// @param maxSanity 最大正気度値
        void SetMaxSanity(float maxSanity);

        /// @brief 最大正気度を取得
        /// @return 最大正気度
        [[nodiscard]] float GetMaxSanity() const;

        /// @brief 正気度を減少させる
        /// @param amount 減少量
        void DecreaseSanity(float amount);

        /// @brief 正気を失っているか確認
        /// @return 正気度が0以下ならtrue
        [[nodiscard]] bool IsInsane() const;

        //============================================================
        // 識別情報
        //============================================================

        /// @brief 名前を設定
        /// @param name キャラクター名
        void SetName(const std::string& name);

        /// @brief 名前を取得
        /// @return キャラクター名
        [[nodiscard]] const std::string& GetName() const;

        //============================================================
        // 部屋・トラップ効果のインターフェース
        //============================================================

        /// @brief 部屋の効果を受ける（派生クラスでオーバーライド可能）
        /// @param room 効果を与える部屋
        virtual void OnRoomEffect(RoomInstance* room);

        /// @brief トラップの効果を受ける（派生クラスでオーバーライド可能）
        /// @param trap 効果を与えるトラップ
        virtual void OnTrapEffect(TrapInstance* trap);

        /// @brief タイルに入った時の処理（派生クラスでオーバーライド可能）
        /// @param tile 入ったタイル
        virtual void OnEnterTile(TileCell* tile);

        /// @brief タイルから出た時の処理（派生クラスでオーバーライド可能）
        /// @param tile 出たタイル
        virtual void OnExitTile(TileCell* tile);

    protected:
        GridPosition m_gridPosition;        ///< グリッド上の位置
        std::string m_name;                 ///< キャラクター名

        float m_health = 100.0f;            ///< 現在の体力
        float m_maxHealth = 100.0f;         ///< 最大体力
        float m_sanity = 100.0f;            ///< 現在の正気度
        float m_maxSanity = 100.0f;         ///< 最大正気度
    };

} // namespace Game
