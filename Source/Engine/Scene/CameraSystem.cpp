/// @file   CameraSystem.cpp
/// @brief  メインカメラの管理を統括するクラス実装
#include "CameraSystem.h"

#include "Engine/Core/Logger.h"
#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/CameraComponent.h"

namespace Engine {

    CameraSystem::~CameraSystem() {
        Finalize();
    }

    //============================================================
    // Lifecycle
    //============================================================

    bool CameraSystem::Initialize(Scene* scene, RenderSystem* renderSystem) {
        if (m_isInitialized) return true;

        if (!scene || !renderSystem) {
            Logger::Error("CameraSystem::Initialize failed: invalid parameters.");
            return false;
        }

        m_scene = scene;
        m_renderSystem = renderSystem;
        m_mainCamera = nullptr;

        m_isInitialized = true;
        Logger::Info("CameraSystem initialized.");
        return true;
    }

    void CameraSystem::Finalize() {
        if (!m_isInitialized) return;

        // カメラGameObjectの破棄はScene側で行われるため、ポインタのクリアのみ
        m_mainCamera = nullptr;
        m_scene = nullptr;
        m_renderSystem = nullptr;

        m_isInitialized = false;
        Logger::Info("CameraSystem finalized.");
    }

    //============================================================
    // Camera Management
    //============================================================

    void CameraSystem::SetMainCamera(CameraComponent* camera) {
        // 以前のメインカメラを非メインに
        if (m_mainCamera && m_mainCamera != camera) {
            m_mainCamera->SetMain(false);
        }

        m_mainCamera = camera;

        if (m_mainCamera) {
            m_mainCamera->SetMain(true);
        }
    }

    CameraComponent* CameraSystem::GetMainCamera() const {
        return m_mainCamera;
    }

    //============================================================
    // Frame
    //============================================================

    void CameraSystem::Update(float deltaTime) {
        (void)deltaTime;
        // 現在、カメラの更新はGameObject/Componentシステム経由で行われるため、
        // ここでは特に追加処理なし。
        // 
        // 将来の拡張予定：
        // - カメラ補間/スムージング
        // - カメラシェイク効果
        // - カメラ切り替えアニメーション
    }

} // namespace Engine
