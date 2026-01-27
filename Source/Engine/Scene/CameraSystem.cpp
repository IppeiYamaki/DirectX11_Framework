/// @file   CameraSystem.cpp
/// @brief  カメラの生成・管理を統括するクラス実装
#include "CameraSystem.h"

#include "Engine/Core/Logger.h"
#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Entity.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/Camera.h"

#include <algorithm>

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
        m_cameraEntities.clear();

        m_isInitialized = true;
        Logger::Info("CameraSystem initialized.");
        return true;
    }

    void CameraSystem::Finalize() {
        if (!m_isInitialized) return;

        // カメラEntityの破棄はScene側で行われるため、ポインタのクリアのみ
        m_mainCamera = nullptr;
        m_cameraEntities.clear();
        m_scene = nullptr;
        m_renderSystem = nullptr;

        m_isInitialized = false;
        Logger::Info("CameraSystem finalized.");
    }

    //============================================================
    // Camera Management
    //============================================================

    Camera* CameraSystem::AddCamera(const CameraInitParams& params) {
        if (!m_isInitialized || !m_scene || !m_renderSystem) {
            Logger::Error("CameraSystem::AddCamera failed: not initialized.");
            return nullptr;
        }

        // カメラ用Entityを生成
        auto* entity = m_scene->CreateEntity("Camera");
        if (!entity) {
            Logger::Error("CameraSystem::AddCamera failed: CreateEntity failed.");
            return nullptr;
        }

        // Transform設定
        if (auto* tr = entity->GetComponent<Transform>()) {
            tr->SetPosition(params.m_position);
            tr->SetYawPitchRollDegrees(params.m_yawDeg, params.m_pitchDeg, params.m_rollDeg);
        }

        // Cameraコンポーネント追加
        auto* camera = entity->AddComponent<Camera>(m_renderSystem);
        if (!camera) {
            Logger::Error("CameraSystem::AddCamera failed: AddComponent<Camera> failed.");
            m_scene->DestroyEntity(entity);
            return nullptr;
        }

        camera->SetPerspective(params.m_fovYRad, params.m_aspect, params.m_nearZ, params.m_farZ);

        // メインカメラとして設定する場合
        if (params.m_isMain) {
            SetMainCamera(camera);
        }

        m_cameraEntities.push_back(entity);
        return camera;
    }

    void CameraSystem::RemoveCamera(Camera* camera) {
        if (!m_isInitialized || !camera || !m_scene) return;

        // カメラを持つEntityを検索
        Entity* targetEntity = camera->GetOwner();
        if (!targetEntity) return;

        // リストから削除
        auto it = std::find(m_cameraEntities.begin(), m_cameraEntities.end(), targetEntity);
        if (it != m_cameraEntities.end()) {
            m_cameraEntities.erase(it);
        }

        // メインカメラの場合はクリア
        if (m_mainCamera == camera) {
            m_mainCamera = nullptr;
        }

        // Entityを破棄
        m_scene->DestroyEntityDeferred(targetEntity);
    }

    void CameraSystem::SetMainCamera(Camera* camera) {
        // 以前のメインカメラを非メインに
        if (m_mainCamera && m_mainCamera != camera) {
            m_mainCamera->SetMain(false);
        }

        m_mainCamera = camera;

        if (m_mainCamera) {
            m_mainCamera->SetMain(true);
        }
    }

    Camera* CameraSystem::GetMainCamera() const {
        return m_mainCamera;
    }

    std::size_t CameraSystem::GetCameraCount() const {
        return m_cameraEntities.size();
    }

    //============================================================
    // Frame
    //============================================================

    void CameraSystem::Update(float deltaTime) {
        (void)deltaTime;
        // カメラの更新はEntity/Componentシステム経由で行われるため、
        // ここでは特に追加処理なし
    }

} // namespace Engine
