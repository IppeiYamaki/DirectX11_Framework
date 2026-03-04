/// @file   CollectibleBallComponent.cpp
/// @brief  収集可能な球体コンポーネント実装
#include "CollectibleBallComponent.h"

#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/LightSystem.h"
#include "Engine/Scene/Light.h"
#include "Game/Gameplay/Maze/MazeGameManager.h"

namespace Game {

    CollectibleBallComponent::CollectibleBallComponent(
        MazeGameManager* gameManager,
        Engine::LightSystem* lightSystem,
        float pickupRadius)
        : m_gameManager(gameManager)
        , m_lightSystem(lightSystem)
        , m_pickupRadius(pickupRadius)
    {
    }

    void CollectibleBallComponent::OnStart() {
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

    void CollectibleBallComponent::Update(float /*deltaTime*/) {
        if (m_isCollected) {
            return;
        }

        CheckPickup();

        // PointLightの位置を更新（オブジェクトが動く場合）
        if (m_pointLight && m_transform) {
            m_pointLight->SetPosition(m_transform->GetPosition());
        }
    }

    void CollectibleBallComponent::OnDestroy() {
        // PointLightを削除
        if (m_pointLight && m_lightSystem) {
            m_lightSystem->RemoveLight(m_pointLight);
            m_pointLight = nullptr;
        }
    }

    void CollectibleBallComponent::SetLightColor(const Engine::Vector3& color) {
        m_lightColor = color;
        if (m_pointLight) {
            m_pointLight->SetColor(color);
        }
    }

    void CollectibleBallComponent::SetLightRange(float range) {
        m_lightRange = range;
        if (m_pointLight) {
            m_pointLight->SetRange(range);
        }
    }

    void CollectibleBallComponent::CheckPickup() {
        if (!m_transform || m_isCollected) {
            return;
        }

        Engine::Vector3 ballPos = m_transform->GetPosition();
        // Y座標を無視して平面距離で判定
        float dx = ballPos.x - m_playerPosition.x;
        float dz = ballPos.z - m_playerPosition.z;
        float distSq = dx * dx + dz * dz;

        if (distSq <= m_pickupRadius * m_pickupRadius) {
            Collect();
        }
    }

    void CollectibleBallComponent::Collect() {
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
