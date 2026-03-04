/// @file   FirstPersonPlayerController.cpp
/// @brief  一人称視点プレイヤーコントローラー実装
#include "FirstPersonPlayerController.h"

#include <DirectXMath.h>

#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Platform/Input.h"

#include <algorithm>
#include <cmath>

namespace Game {

    FirstPersonPlayerController::FirstPersonPlayerController(
        const MazeGrid* grid,
        const FirstPersonPlayerSettings& settings,
        float cellSize)
        : m_grid(grid)
        , m_settings(settings)
        , m_cellSize(cellSize)
    {
    }

    void FirstPersonPlayerController::OnStart() {
        auto* owner = GetOwner();
        if (owner) {
            m_transform = owner->GetComponent<Engine::Transform>();

            if (m_transform) {
                const Engine::Vector3& rotation = m_transform->GetRotationEulerDegrees();
                m_yaw = rotation.y;
                m_pitch = rotation.x;
            }
        }
    }

    void FirstPersonPlayerController::Update(float deltaTime) {
        if (!m_isControlEnabled || !m_transform) {
            return;
        }

        ProcessRotation();
        ProcessMovement(deltaTime);
    }

    Engine::Vector3 FirstPersonPlayerController::GetPlayerPosition() const {
        if (!m_transform) {
            return Engine::Vector3::Zero();
        }
        Engine::Vector3 pos = m_transform->GetPosition();
        pos.y = 0.0f;
        return pos;
    }

    void FirstPersonPlayerController::ProcessRotation() {
        // マウス移動量を取得
        POINT mouseDelta = Engine::Input::GetMouseDelta();

        if (mouseDelta.x == 0 && mouseDelta.y == 0) {
            return;
        }

        // Yaw（水平回転）とPitch（垂直回転）を更新
        m_yaw += static_cast<float>(mouseDelta.x) * m_settings.m_mouseSensitivity;
        m_pitch += static_cast<float>(mouseDelta.y) * m_settings.m_mouseSensitivity;

        // Pitch角度を制限
        m_pitch = std::clamp(m_pitch, m_settings.m_pitchLimitMin, m_settings.m_pitchLimitMax);

        // Yawを正規化
        m_yaw = std::fmod(m_yaw, 360.0f);
        if (m_yaw < 0.0f) m_yaw += 360.0f;

        // Transformに回転を適用
        m_transform->SetYawPitchRollDegrees(m_yaw, m_pitch, 0.0f);
    }

    void FirstPersonPlayerController::ProcessMovement(float deltaTime) {
        Engine::Vector3 moveDir = Engine::Vector3::Zero();

        // WASDキー入力を取得
        if (Engine::Input::IsKeyPressed('W')) {
            moveDir.z += 1.0f;
        }
        if (Engine::Input::IsKeyPressed('S')) {
            moveDir.z -= 1.0f;
        }
        if (Engine::Input::IsKeyPressed('A')) {
            moveDir.x -= 1.0f;
        }
        if (Engine::Input::IsKeyPressed('D')) {
            moveDir.x += 1.0f;
        }

        // 移動量がなければ終了
        if (moveDir.LengthSquared() < 0.001f) {
            return;
        }

        // 移動方向を正規化
        moveDir.NormalizeInPlace();

        // カメラのYaw角度に基づいて移動方向を回転
        float yawRad = m_yaw * DirectX::XM_PI / 180.0f;
        float sinYaw = std::sin(yawRad);
        float cosYaw = std::cos(yawRad);

        // XZ平面での移動ベクトルを計算
        Engine::Vector3 worldMoveDir;
        worldMoveDir.x = moveDir.x * cosYaw + moveDir.z * sinYaw;
        worldMoveDir.y = 0.0f;
        worldMoveDir.z = -moveDir.x * sinYaw + moveDir.z * cosYaw;

        // 移動量を計算
        Engine::Vector3 movement = worldMoveDir * m_settings.m_moveSpeed * deltaTime;

        // 現在位置と希望位置を取得
        Engine::Vector3 currentPos = m_transform->GetPosition();
        Engine::Vector3 desiredPos = currentPos + movement;

        // 壁との衝突を考慮した最終位置を計算
        Engine::Vector3 finalPos = ApplyWallCollision(currentPos, desiredPos);

        // 位置を更新（Y座標は固定）
        finalPos.y = m_settings.m_eyeHeight;
        m_transform->SetPosition(finalPos);
    }

    Engine::Vector3 FirstPersonPlayerController::ApplyWallCollision(
        const Engine::Vector3& currentPos,
        const Engine::Vector3& desiredPos)
    {
        if (!m_grid) {
            return desiredPos;
        }

        Engine::Vector3 finalPos = desiredPos;
        float radius = m_settings.m_playerRadius;

        // X方向の移動をチェック
        if (!CheckWallCollision(desiredPos.x, currentPos.z)) {
            // X方向は安全
        } else {
            // X方向は衝突、元の位置に戻す
            finalPos.x = currentPos.x;
        }

        // Z方向の移動をチェック
        if (!CheckWallCollision(finalPos.x, desiredPos.z)) {
            // Z方向は安全
        } else {
            // Z方向は衝突、元の位置に戻す
            finalPos.z = currentPos.z;
        }

        // 最終位置を再度チェック（角の衝突対策）
        if (CheckWallCollision(finalPos.x, finalPos.z)) {
            finalPos = currentPos;
        }

        return finalPos;
    }

    bool FirstPersonPlayerController::CheckWallCollision(float worldX, float worldZ) {
        if (!m_grid) {
            return false;
        }

        float radius = m_settings.m_playerRadius;

        // プレイヤーの四隅をチェック
        float checkPoints[4][2] = {
            { worldX - radius, worldZ - radius },
            { worldX + radius, worldZ - radius },
            { worldX - radius, worldZ + radius },
            { worldX + radius, worldZ + radius }
        };

        for (int i = 0; i < 4; ++i) {
            int gridX, gridZ;
            MazeGrid::WorldToGrid(checkPoints[i][0], checkPoints[i][1], m_cellSize, gridX, gridZ);
            
            if (!m_grid->IsWalkable(gridX, gridZ)) {
                return true;
            }
        }

        return false;
    }

} // namespace Game
