/// @file   CameraPrefab.h
/// @brief  CameraComponentを持つGameObjectを生成するPrefabクラス
#pragma once

#include <DirectXMath.h>

#include "Engine/Math/Vector3.h"


namespace Engine { 
    struct SceneContext;
    class GameObject;
    class CameraComponent;
}

namespace Game {

    /// @brief Cameraオブジェクトを生成するPrefab
    /// @note  CameraComponent + Transform を持つGameObjectを生成
    class CameraPrefab_GameSceneMain final {
    public:
        /// @brief 生成パラメータ
        struct SpawnDesc final {
            Engine::Vector3 m_position{ 0.0f, 0.0f, -10.0f };   /// カメラの初期位置
            float m_yawDeg = 0.0f;                       /// Yaw角度（度）
            float m_pitchDeg = 0.0f;                     /// Pitch角度（度）
            float m_rollDeg = 0.0f;                      /// Roll角度（度）
            float m_fovYRad = DirectX::XM_PIDIV4;        /// 垂直FOV（ラジアン）
            float m_aspect = 16.0f / 9.0f;               /// アスペクト比
            float m_nearZ = 0.1f;                        /// 近クリップ面
            float m_farZ = 1000.0f;                      /// 遠クリップ面
            bool m_isMain = true;                        /// メインカメラに設定するか

            SpawnDesc() = default;

            /// @brief 位置と回転を指定するコンストラクタ
            /// @param position カメラ位置
            /// @param yawDeg Yaw角度（度）
            /// @param pitchDeg Pitch角度（度）
            SpawnDesc(const Engine::Vector3& position,
                      float yawDeg = 0.0f,
                      float pitchDeg = 0.0f)
                : m_position(position)
                , m_yawDeg(yawDeg)
                , m_pitchDeg(pitchDeg) {
            }

            /// @brief フルパラメータコンストラクタ
            SpawnDesc(const Engine::Vector3& position,
                      float yawDeg,
                      float pitchDeg,
                      float rollDeg,
                      float fovYRad,
                      float aspect,
                      float nearZ,
                      float farZ,
                      bool isMain = true)
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

        /// @brief Cameraオブジェクトを生成
        /// @param ctx SceneContext
        /// @param desc 生成パラメータ
        /// @return 生成されたGameObject（CameraComponentを保持）
        static Engine::GameObject* Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc);

        /// @brief 生成されたCameraからCameraComponentを取得
        /// @param cameraObject Spawn()で生成されたGameObject
        /// @return CameraComponentへのポインタ（見つからなければnullptr）
        static Engine::CameraComponent* GetCameraComponent(Engine::GameObject* cameraObject);
    };

} // namespace Engine
