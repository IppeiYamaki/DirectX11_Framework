/// @file   CameraSystem.h
/// @brief  メインカメラの管理を統括するクラス
/// @note   カメラの生成はCameraPrefabを使用し、このクラスはメインカメラの管理のみを担当
#pragma once

#include <DirectXMath.h>

#include "Engine/Math/Vector3.h"

namespace Engine {

    class RenderSystem;
    class CameraComponent;
    class GameObject;
    class Scene;

    // Backward compatibility alias
    using Camera = CameraComponent;

    /// @brief カメラ初期化パラメータ
    /// @deprecated CameraPrefab::SpawnDescを使用してください
    struct CameraInitParams final {
        Vector3 m_position{ 0.0f, 0.0f, -10.0f };   ///< カメラの初期位置
        float m_yawDeg = 0.0f;                       ///< Yaw角度（度）
        float m_pitchDeg = 0.0f;                     ///< Pitch角度（度）
        float m_rollDeg = 0.0f;                      ///< Roll角度（度）
        float m_fovYRad = DirectX::XM_PIDIV4;        ///< 垂直FOV（ラジアン）
        float m_aspect = 16.0f / 9.0f;               ///< アスペクト比
        float m_nearZ = 0.1f;                        ///< 近クリップ面
        float m_farZ = 1000.0f;                      ///< 遠クリップ面
        bool m_isMain = false;                       ///< メインカメラに設定するか

        CameraInitParams() = default;

        /// @brief コンストラクタ
        CameraInitParams(const Vector3& position,
            float yawDeg = 0.0f,
            float pitchDeg = 0.0f,
            float rollDeg = 0.0f,
            float fovYRad = DirectX::XM_PIDIV4,
            float aspect = 16.0f / 9.0f,
            float nearZ = 0.1f,
            float farZ = 1000.0f,
            bool isMain = false)
            : m_position(position)
            , m_yawDeg(yawDeg)
            , m_pitchDeg(pitchDeg)
            , m_rollDeg(rollDeg)
            , m_fovYRad(fovYRad)
            , m_aspect(aspect)
            , m_nearZ(nearZ)
            , m_farZ(farZ)
            , m_isMain(isMain) {
        }
    };

    /// @brief メインカメラの管理を統括するシステム
    /// @note  メインカメラの管理のみを担当。カメラの生成はCameraPrefabを使用
    class CameraSystem final {
    public:
        CameraSystem() = default;
        ~CameraSystem();

        CameraSystem(const CameraSystem&) = delete;
        CameraSystem& operator=(const CameraSystem&) = delete;

        //============================================================
        // Lifecycle
        //============================================================

        /// @brief  初期化
        /// @param  scene シーン参照
        /// @param  renderSystem RenderSystem参照
        /// @return 成功ならtrue
        [[nodiscard]] bool Initialize(Scene* scene, RenderSystem* renderSystem);

        /// @brief 終了処理
        void Finalize();

        //============================================================
        // Camera Management
        //============================================================

        /// @brief  メインカメラを設定
        /// @param  camera メインカメラに設定するカメラ
        void SetMainCamera(CameraComponent* camera);

        /// @brief  メインカメラを取得
        /// @return メインカメラへのポインタ
        [[nodiscard]] CameraComponent* GetMainCamera() const;

        //============================================================
        // Frame
        //============================================================

        /// @brief 各カメラを更新
        /// @param deltaTime フレーム経過時間
        void Update(float deltaTime);

    private:
        Scene* m_scene = nullptr;                       ///< シーン参照（借用）
        RenderSystem* m_renderSystem = nullptr;         ///< RenderSystem参照（借用）
        CameraComponent* m_mainCamera = nullptr;        ///< メインカメラへのポインタ（借用）
        bool m_isInitialized = false;                   ///< 初期化済みフラグ
    };

} // namespace Engine
