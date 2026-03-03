/// @file   TrapInstance.h
/// @brief  トラップインスタンスの基底クラス
#pragma once

#include "Game/Gameplay/GridPosition.h"
#include <string>

namespace Game {

    class CharacterInstance;

    /// @brief トラップの種類
    enum class TrapType {
        Damage,         ///< ダメージを与えるトラップ
        Slow,           ///< 移動速度を低下させるトラップ
        Stun,           ///< 行動を停止させるトラップ
        SanityDrain,    ///< 正気度を減少させるトラップ
        Custom          ///< カスタムトラップ
    };

    /// @brief トラップインスタンスの基底クラス
    /// @note  各タイルに複数配置可能なトラップの基底クラス
    class TrapInstance {
    public:
        /// @brief コンストラクタ
        /// @param name トラップ名
        /// @param type トラップの種類
        explicit TrapInstance(const std::string& name = "Trap", TrapType type = TrapType::Damage);

        /// @brief デストラクタ
        virtual ~TrapInstance() = default;

        // コピー禁止
        TrapInstance(const TrapInstance&) = delete;
        TrapInstance& operator=(const TrapInstance&) = delete;

        //============================================================
        // 識別情報
        //============================================================

        /// @brief 名前を設定
        /// @param name トラップ名
        void SetName(const std::string& name);

        /// @brief 名前を取得
        /// @return トラップ名
        [[nodiscard]] const std::string& GetName() const;

        /// @brief トラップの種類を取得
        /// @return トラップの種類
        [[nodiscard]] TrapType GetTrapType() const;

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

        /// @brief トラップが有効か確認
        /// @return 有効ならtrue
        [[nodiscard]] bool IsActive() const;

        /// @brief トラップを有効化
        void Activate();

        /// @brief トラップを無効化
        void Deactivate();

        /// @brief 使用回数制限を設定
        /// @param uses 使用可能回数（-1で無制限）
        void SetMaxUses(int uses);

        /// @brief 残り使用回数を取得
        /// @return 残り使用回数（-1で無制限）
        [[nodiscard]] int GetRemainingUses() const;

        /// @brief 使い果たしたか確認
        /// @return 使い果たしていればtrue
        [[nodiscard]] bool IsExhausted() const;

        //============================================================
        // キャラクター効果のインターフェース
        //============================================================

        /// @brief トラップを発動
        /// @param character 対象キャラクター
        /// @return 発動成功ならtrue
        virtual bool Trigger(CharacterInstance* character);

        /// @brief トラップの効果をキャラクターに適用
        /// @param character 対象キャラクター
        /// @note  全てのトラップタイプでキャラクターのOnTrapEffectが呼び出される
        ///        派生クラスの効果（Slow, Stun, Custom）は派生クラスで実装する
        virtual void ApplyEffectToCharacter(CharacterInstance* character);

        //============================================================
        // 効果パラメータ
        //============================================================

        /// @brief ダメージ量を設定
        /// @param damage ダメージ量
        void SetDamage(float damage);

        /// @brief ダメージ量を取得
        /// @return ダメージ量
        [[nodiscard]] float GetDamage() const;

        /// @brief 正気度減少量を設定
        /// @param sanityDrain 正気度減少量
        void SetSanityDrain(float sanityDrain);

        /// @brief 正気度減少量を取得
        /// @return 正気度減少量
        [[nodiscard]] float GetSanityDrain() const;

        /// @brief スタンターン数を設定
        /// @param turns スタンターン数
        void SetStunTurns(int turns);

        /// @brief スタンターン数を取得
        /// @return スタンターン数
        [[nodiscard]] int GetStunTurns() const;

    protected:
        std::string m_name;             ///< トラップ名
        TrapType m_type;                ///< トラップの種類
        GridPosition m_gridPosition;   ///< グリッド上の位置

        bool m_isActive = true;         ///< 有効状態
        int m_maxUses = -1;             ///< 最大使用回数（-1で無制限）
        int m_remainingUses = -1;       ///< 残り使用回数

        // 効果パラメータ
        float m_damage = 10.0f;         ///< ダメージ量
        float m_sanityDrain = 5.0f;     ///< 正気度減少量
        int m_stunTurns = 1;            ///< スタンターン数
    };

} // namespace Game
