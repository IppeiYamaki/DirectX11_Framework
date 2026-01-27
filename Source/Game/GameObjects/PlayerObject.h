/// @file   PlayerObject.h
/// @brief  プレイヤーキャラクターのGameObject派生クラス
#pragma once

#include "Engine/Scene/GameObject.h"
#include "Engine/Math/Vector3.h"

namespace Game {

    /// @brief プレイヤーキャラクター用のGameObject派生クラス
    /// @note  プレイヤー固有の挙動やスクリプトロジックをこのクラス内で自己完結的に記述
    class PlayerObject : public Engine::GameObject {
    public:
        /// @brief コンストラクタ
        /// @param name オブジェクト名（デフォルト: "Player"）
        explicit PlayerObject(const std::string& name = "Player");

        /// @brief デストラクタ
        ~PlayerObject() override = default;

        //============================================================
        // ライフサイクルオーバーライド
        //============================================================

        /// @brief 初期化処理
        void Initialize() override;

        /// @brief 毎フレームの更新処理
        /// @param deltaTime 前フレームからの経過時間（秒）
        void Update(float deltaTime) override;

        /// @brief 描画処理
        void Render() override;

        /// @brief 破棄処理
        void OnDestroy() override;

        //============================================================
        // プレイヤー固有機能
        //============================================================

        /// @brief 移動速度を設定
        /// @param speed 移動速度
        void SetMoveSpeed(float speed);

        /// @brief 移動速度を取得
        /// @return 現在の移動速度
        [[nodiscard]] float GetMoveSpeed() const;

        /// @brief HPを設定
        /// @param hp HP値
        void SetHealth(float hp);

        /// @brief HPを取得
        /// @return 現在のHP
        [[nodiscard]] float GetHealth() const;

        /// @brief ダメージを受ける
        /// @param damage ダメージ量
        void TakeDamage(float damage);

        /// @brief 生存しているか確認
        /// @return 生存していればtrue
        [[nodiscard]] bool IsAlive() const;

    protected:
        /// @brief 入力処理（派生クラスでオーバーライド可能）
        virtual void HandleInput(float deltaTime);

        /// @brief 移動処理
        /// @param direction 移動方向（正規化済み）
        /// @param deltaTime フレーム経過時間
        void Move(const Engine::Vector3& direction, float deltaTime);

    private:
        float m_moveSpeed = 5.0f;   ///< 移動速度
        float m_health = 100.0f;    ///< HP
        float m_maxHealth = 100.0f; ///< 最大HP
    };

} // namespace Game
