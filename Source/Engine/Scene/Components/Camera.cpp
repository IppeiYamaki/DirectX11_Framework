#include "Camera.h"

#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Physics/Ray.h"
#include "Engine/Math/Vector3.h"

namespace Engine {

    Camera::Camera(RenderSystem* renderSystem)
        : m_renderSystem(renderSystem) {
    }

    void Camera::OnStart() {
        // 初期化時にすぐに適用
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

        // main切り替え直後にすぐ適用
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

    Ray Camera::ScreenPointToRay(float screenX, float screenY, float screenWidth, float screenHeight) const {
        // スクリーン座標をNDC（正規化デバイス座標）に変換
        // NDC: X[-1,1], Y[-1,1], Z[0,1]（DirectX）
        float ndcX = (2.0f * screenX / screenWidth) - 1.0f;
        float ndcY = 1.0f - (2.0f * screenY / screenHeight); // Yは反転

        // 投影行列を取得
        DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(m_fovY, m_aspect, m_nearZ, m_farZ);
        DirectX::XMMATRIX invP = DirectX::XMMatrixInverse(nullptr, P);

        // ビュー行列を取得
        auto* owner = GetOwner();
        DirectX::XMMATRIX invV = DirectX::XMMatrixIdentity();
        Vector3 cameraPos = Vector3::Zero();

        if (owner) {
            auto* tr = owner->GetComponent<Engine::Transform>();
            if (tr) {
                const auto& world = tr->GetWorldMatrix();
                DirectX::XMMATRIX W = DirectX::XMLoadFloat4x4(&world);
                // View = inverse(World)なので、World = inverse(View)
                invV = W;

                // カメラ位置を取得
                cameraPos = Vector3(world._41, world._42, world._43);
            }
        }

        // ニアプレーン上の点をビュー空間に変換
        DirectX::XMVECTOR nearPointNDC = DirectX::XMVectorSet(ndcX, ndcY, 0.0f, 1.0f);
        DirectX::XMVECTOR nearPointView = DirectX::XMVector4Transform(nearPointNDC, invP);

        // 同次座標からの変換
        DirectX::XMFLOAT4 nearViewF4;
        DirectX::XMStoreFloat4(&nearViewF4, nearPointView);
        float w = nearViewF4.w;
        if (std::abs(w) > 0.0001f) {
            nearViewF4.x /= w;
            nearViewF4.y /= w;
            nearViewF4.z /= w;
        }

        // ビュー空間での方向ベクトル（カメラ原点から点へ）
        DirectX::XMVECTOR dirView = DirectX::XMVectorSet(nearViewF4.x, nearViewF4.y, nearViewF4.z, 0.0f);
        dirView = DirectX::XMVector3Normalize(dirView);

        // ワールド空間に変換
        DirectX::XMVECTOR dirWorld = DirectX::XMVector3TransformNormal(dirView, invV);
        dirWorld = DirectX::XMVector3Normalize(dirWorld);

        DirectX::XMFLOAT3 dirF3;
        DirectX::XMStoreFloat3(&dirF3, dirWorld);

        Vector3 direction(dirF3.x, dirF3.y, dirF3.z);

        return Ray(cameraPos, direction);
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
