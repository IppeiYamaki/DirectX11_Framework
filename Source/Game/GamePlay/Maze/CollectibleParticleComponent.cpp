/// @file   CollectibleParticleComponent.cpp
/// @brief  収集可能な蛍パーティクルコンポーネント実装
#include "CollectibleParticleComponent.h"

#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/LightSystem.h"
#include "Engine/Scene/Light.h"
#include "Game/Gameplay/Maze/MazeGameManager.h"
#include "Game/Gameplay/Maze/MazeGrid.h"

namespace Game {

    CollectibleParticleComponent::CollectibleParticleComponent(
        MazeGameManager* gameManager,
        Engine::LightSystem* lightSystem,
        int cellX, int cellZ,
        float cellSize)
        : m_gameManager(gameManager)
        , m_lightSystem(lightSystem)
        , m_cellX(cellX)
        , m_cellZ(cellZ)
        , m_cellSize(cellSize)
    {
    }

    void CollectibleParticleComponent::OnStart() {
        auto* owner = GetOwner();
        if (owner) {
            m_transform = owner->GetComponent<Engine::Transform>();
        }

        // PointLightを追加
        if (m_lightSystem && m_transform) {
            Engine::Vector3 pos = m_transform->GetPosition();
            m_pointLight = m_lightSystem->AddPointLight(pos);
            if (m_pointLight) {
                m_pointLight->SetColor(m_lightColor);
                m_pointLight->SetRange(m_lightRange);
                m_pointLight->SetIntensity(m_lightIntensity);
            }
        }
    }

    void CollectibleParticleComponent::Update(float /*deltaTime*/) {
        if (m_isCollected) {
            return;
        }

        CheckPickup();
        UpdateLightPosition();
    }

    void CollectibleParticleComponent::OnDestroy() {
        // PointLightを削除
        if (m_pointLight && m_lightSystem) {
            m_lightSystem->RemoveLight(m_pointLight);
            m_pointLight = nullptr;
        }
    }

    void CollectibleParticleComponent::SetLightColor(const Engine::Vector3& color) {
        m_lightColor = color;
        if (m_pointLight) {
            m_pointLight->SetColor(color);
        }
    }

    void CollectibleParticleComponent::SetLightRange(float range) {
        m_lightRange = range;
        if (m_pointLight) {
            m_pointLight->SetRange(range);
        }
    }

    void CollectibleParticleComponent::SetLightIntensity(float intensity) {
        m_lightIntensity = intensity;
        if (m_pointLight) {
            m_pointLight->SetIntensity(intensity);
        }
    }

    void CollectibleParticleComponent::CheckPickup() {
        if (m_isCollected) {
            return;
        }

        // プレイヤーのワールド座標からグリッド座標を計算
        int playerCellX, playerCellZ;
        MazeGrid::WorldToGrid(m_playerPosition.x, m_playerPosition.z, m_cellSize, playerCellX, playerCellZ);

        // グリッド座標が一致したら取得
        if (playerCellX == m_cellX && playerCellZ == m_cellZ) {
            Collect();
        }
    }

    void CollectibleParticleComponent::UpdateLightPosition() {
        // PointLightの位置をパーティクル（ふわふわ移動）に追従
        if (m_pointLight && m_transform) {
            m_pointLight->SetPosition(m_transform->GetPosition());
        }
    }

    void CollectibleParticleComponent::Collect() {
        m_isCollected = true;

        // PointLightを削除
        if (m_pointLight && m_lightSystem) {
            m_lightSystem->RemoveLight(m_pointLight);
            m_pointLight = nullptr;
        }

        // GameObjectを非アクティブ化
        if (auto* owner = GetOwner()) {
            owner->SetActive(false);
        }

        // GameManagerに通知
        if (m_gameManager) {
            m_gameManager->OnBallCollected();
        }
    }

} // namespace Game
