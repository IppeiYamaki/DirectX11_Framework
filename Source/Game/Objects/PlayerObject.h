/// @file   PlayerObject.h
/// @brief  プレイヤーキャラクターのGameObject
#pragma once

#include "Engine/Scene/GameObject.h"

namespace Game {

    /// @brief プレイヤーキャラクターのGameObject
    /// @note  プレイヤー固有の振る舞いを自己完結的に記述
    class PlayerObject : public Engine::GameObject {
    public:
        /// @brief コンストラクタ
        /// @param name オブジェクト名
        explicit PlayerObject(const std::string& name = "Player");

        /// @brief デストラクタ
        ~PlayerObject() override = default;

        //============================================================
        // Lifecycle
        //============================================================

        /// @brief 初期化処理
        void Initialize() override;

        /// @brief 毎フレームの処理
        /// @param deltaTime フレーム経過時間（秒）
        void Update(float deltaTime) override;

        /// @brief 描画処理
        void Render() override;

        //============================================================
        // Player Specific
        //============================================================

        /// @brief 移動速度を設定
        /// @param speed 移動速度
        void SetMoveSpeed(float speed);

        /// @brief 移動速度を取得
        /// @return 移動速度
        [[nodiscard]] float GetMoveSpeed() const;

        /// @brief HPを設定
        /// @param hp HP値
        void SetHP(int hp);

        /// @brief HPを取得
        /// @return HP値
        [[nodiscard]] int GetHP() const;

        /// @brief ダメージを受ける
        /// @param damage ダメージ量
        void TakeDamage(int damage);

        /// @brief 死亡しているか確認
        /// @return 死亡していればtrue
        [[nodiscard]] bool IsDead() const;

    private:
        /// @brief 入力処理
        /// @param deltaTime フレーム経過時間
        void HandleInput(float deltaTime);

    private:
        float m_moveSpeed = 5.0f;       ///< 移動速度
        int m_hp = 100;                 ///< HP
        int m_maxHP = 100;              ///< 最大HP
    };

} // namespace Game
