#include "Camera.h"

#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"

namespace Engine {

    Camera::Camera(RenderSystem* renderSystem)
        : m_renderSystem(renderSystem) {
    }

    void Camera::OnStart() {
        // ���������t���[�����炷�����f�������̂�1��K�p
        if (m_isMain) {
            ApplyToRenderSystem();
        }
    }

    void Camera::LateUpdate(float deltaTime) {
        (void)deltaTime;

        if (!m_isMain) return;
        ApplyToRenderSystem();
    }

    void Camera::SetMain(bool isMain) {
        m_isMain = isMain;

        // main�؂�ւ�����ɑ����f�i���S�j
        if (m_isMain) {
            ApplyToRenderSystem();
        }
    }

    bool Camera::IsMain() const {
        return m_isMain;
    }

    void Camera::SetPerspective(float fovYRadians, float aspect, float nearZ, float farZ) {
        m_fovY = fovYRadians;
        m_aspect = aspect;
        m_nearZ = nearZ;
        m_farZ = farZ;

        if (m_isMain) {
            ApplyToRenderSystem();
        }
    }

    void Camera::ApplyToRenderSystem() {
        if (!m_renderSystem) return;

        auto* owner = GetOwner();
        if (!owner) return;

        auto* tr = owner->GetComponent<Engine::Transform>();
        if (!tr) return;

        // View = inverse(World)
        const auto& world = tr->GetWorldMatrix();
        DirectX::XMMATRIX W = DirectX::XMLoadFloat4x4(&world);
        DirectX::XMMATRIX V = DirectX::XMMatrixInverse(nullptr, W);

        DirectX::XMFLOAT4X4 view{};
        DirectX::XMStoreFloat4x4(&view, V);

        // Projection
        DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(m_fovY, m_aspect, m_nearZ, m_farZ);
        DirectX::XMFLOAT4X4 proj{};
        DirectX::XMStoreFloat4x4(&proj, P);

        m_renderSystem->SetViewMatrix(view);
        m_renderSystem->SetProjectionMatrix(proj);
    }

} // namespace Engine
