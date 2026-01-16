#pragma once

#include <DirectXMath.h>

#include "Engine/Scene/Component.h"

namespace Engine {

    class RenderSystem;

    /**
     * @brief MainCamera（View/ProjectionをRenderSystemへ反映）
     * - Transform の WorldMatrix の逆行列を View として使う
     * - LateUpdate で毎フレーム反映（追従カメラ等に強い）
     */
    class Camera final : public Component {
    public:
        explicit Camera(RenderSystem* renderSystem);
        ~Camera() override = default;

        void OnStart() override;
        void LateUpdate(float deltaTime) override;

        void SetMain(bool isMain);
        bool IsMain() const;

        void SetPerspective(float fovYRadians, float aspect, float nearZ, float farZ);

    private:
        void ApplyToRenderSystem();

    private:
        RenderSystem* m_renderSystem = nullptr; // non-owning
        bool m_isMain = false;

        float m_fovY = DirectX::XM_PIDIV4; // 45deg
        float m_aspect = 16.0f / 9.0f;
        float m_nearZ = 0.1f;
        float m_farZ = 1000.0f;
    };

} // namespace Engine
