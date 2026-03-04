/// @file   CollectibleBallComponent.h
/// @brief  収集可能な球体コンポーネント
#pragma once

#include "Engine/Scene/Component.h"
#include "Engine/Math/Vector3.h"

namespace Engine {
    class Transform;
    class PointLightObject;
    class LightSystem;
}

namespace Game {

    class MazeGameManager;

    /// @brief 収集可能な球体コンポーネント
    class CollectibleBallComponent final : public Engine::Component {
    public:
        /// @brief コンストラクタ
        /// @param gameManager ゲームマネージャー（取得時の通知用）
        /// @param lightSystem ライトシステム（PointLight管理用）
        /// @param pickupRadius 取得判定半径
        CollectibleBallComponent(MazeGameManager* gameManager, Engine::LightSystem* lightSystem, float pickupRadius);

        ~CollectibleBallComponent() override = default;

        //============================================================
        // ライフサイクル
        //============================================================

        void OnStart() override;
        void Update(float deltaTime) override;
        void OnDestroy() override;

        //============================================================
        // 状態
        //============================================================

        /// @brief 取得済みかチェック
        [[nodiscard]] bool IsCollected() const { return m_isCollected; }

        /// @brief プレイヤー位置を設定（毎フレームGameManagerから更新）
        void SetPlayerPosition(const Engine::Vector3& pos) { m_playerPosition = pos; }

        /// @brief PointLightの色を設定
        void SetLightColor(const Engine::Vector3& color);

        /// @brief PointLightの範囲を設定
        void SetLightRange(float range);

    private:
        /// @brief プレイヤーとの距離をチェックして取得判定
        void CheckPickup();

        /// @brief 取得処理
        void Collect();

    private:
        MazeGameManager* m_gameManager = nullptr;
        Engine::LightSystem* m_lightSystem = nullptr;
        Engine::Transform* m_transform = nullptr;
        Engine::PointLightObject* m_pointLight = nullptr;

        Engine::Vector3 m_playerPosition;
        float m_pickupRadius = 1.0f;
        bool m_isCollected = false;

        // ライト設定
        Engine::Vector3 m_lightColor{ 1.0f, 0.8f, 0.3f };  // 暖色
        float m_lightRange = 4.0f;
        float m_lightIntensity = 1.5f;
    };

} // namespace Game
