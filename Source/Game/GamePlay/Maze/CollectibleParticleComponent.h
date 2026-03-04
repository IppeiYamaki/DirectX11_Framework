/// @file   CollectibleParticleComponent.h
/// @brief  収集可能な蛍パーティクルコンポーネント
#pragma once

#include "Engine/Scene/Component.h"
#include "Engine/Math/Vector3.h"

namespace Engine {
    class Transform;
    class PointLightObject;
    class LightSystem;
    class GameObject;
}

namespace Game {

    class MazeGameManager;

    /// @brief 収集可能な蛍パーティクルコンポーネント
    /// @note  グリッド判定方式で取得判定を行い、PointLightを持つ
    class CollectibleParticleComponent final : public Engine::Component {
    public:
        /// @brief コンストラクタ
        /// @param gameManager ゲームマネージャー（取得時の通知用）
        /// @param lightSystem ライトシステム（PointLight管理用）
        /// @param cellX 所属セルX座標
        /// @param cellZ 所属セルZ座標
        /// @param cellSize セルサイズ
        CollectibleParticleComponent(
            MazeGameManager* gameManager,
            Engine::LightSystem* lightSystem,
            int cellX, int cellZ,
            float cellSize);

        ~CollectibleParticleComponent() override = default;

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

        /// @brief PointLightの強度を設定
        void SetLightIntensity(float intensity);

        /// @brief 所属セルX座標を取得
        [[nodiscard]] int GetCellX() const { return m_cellX; }

        /// @brief 所属セルZ座標を取得
        [[nodiscard]] int GetCellZ() const { return m_cellZ; }

    private:
        /// @brief プレイヤーとの距離またはグリッドをチェックして取得判定
        void CheckPickup();

        /// @brief 取得処理
        void Collect();

        /// @brief PointLightの位置を更新（パーティクル位置に追従）
        void UpdateLightPosition();

    private:
        MazeGameManager* m_gameManager = nullptr;
        Engine::LightSystem* m_lightSystem = nullptr;
        Engine::Transform* m_transform = nullptr;
        Engine::PointLightObject* m_pointLight = nullptr;

        Engine::Vector3 m_playerPosition;
        int m_cellX = 0;           // 所属セルX（グリッド座標）
        int m_cellZ = 0;           // 所属セルZ（グリッド座標）
        float m_cellSize = 2.0f;   // セルサイズ
        bool m_isCollected = false;

        // ライト設定
        Engine::Vector3 m_lightColor{ 1.0f, 0.7f, 0.2f };  // 暖色
        float m_lightRange = 4.0f;
        float m_lightIntensity = 2.0f;
    };

} // namespace Game
