#include "PlanarReflectionSystem.h"

#include <string>

#include "Engine/Core/Logger.h"
#include "Engine/Graphics/GraphicsDevice.h"

namespace Engine {

    PlanarReflectionSystem::~PlanarReflectionSystem() {
        Finalize();
    }

    bool PlanarReflectionSystem::Initialize(GraphicsDevice* device, std::uint32_t width, std::uint32_t height) {
        if (!device) {
            Logger::Error("PlanarReflectionSystem::Initialize failed: device is null.");
            return false;
        }

        if (m_isInitialized) {
            Finalize();
        }

        m_device = device;

        // 反射レンダーターゲットを作成
        m_reflectionRT = std::make_unique<RenderTarget>();
        if (!m_reflectionRT->Create(device->GetDevice(), width, height)) {
            Logger::Error("PlanarReflectionSystem::Initialize failed: could not create reflection RT.");
            Finalize();
            return false;
        }

        // デフォルトの反射平面を設定（Y=0の水平面）
        m_plane = ReflectionPlane(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
        UpdateReflectionMatrix();

        m_isInitialized = true;
		Logger::Info("PlanarReflectionSystem initialized: " + std::to_string(width) + "x" + std::to_string(height) + " reflection RT created.");
        return true;
    }

    void PlanarReflectionSystem::Finalize() {
        m_reflectionRT.reset();
        m_device = nullptr;
        m_isInitialized = false;
    }

    bool PlanarReflectionSystem::IsInitialized() const {
        return m_isInitialized;
    }

    void PlanarReflectionSystem::SetReflectionPlane(const ReflectionPlane& plane) {
        m_plane = plane;
        m_plane.m_normal = plane.m_normal.Normalized();
        UpdateReflectionMatrix();
    }

    const ReflectionPlane& PlanarReflectionSystem::GetReflectionPlane() const {
        return m_plane;
    }

    void PlanarReflectionSystem::UpdateReflectionMatrix() {
        // 平面方程式: ax + by + cz + d = 0
        // ここで (a,b,c) = 法線, d = -dot(法線, 平面上の点)
        float a = m_plane.m_normal.x;
        float b = m_plane.m_normal.y;
        float c = m_plane.m_normal.z;
        float d = -Vector3::Dot(m_plane.m_normal, m_plane.m_point);

        // 反射行列の構築
        // R = I - 2 * n * n^T - 2 * d * n
        // ただし行列形式で：
        // | 1-2a² , -2ab  , -2ac  , -2ad |
        // | -2ab  , 1-2b² , -2bc  , -2bd |
        // | -2ac  , -2bc  , 1-2c² , -2cd |
        // | 0     , 0     , 0     , 1    |

        DirectX::XMMATRIX reflectMat = DirectX::XMMatrixSet(
            1.0f - 2.0f * a * a, -2.0f * a * b, -2.0f * a * c, 0.0f,
            -2.0f * a * b, 1.0f - 2.0f * b * b, -2.0f * b * c, 0.0f,
            -2.0f * a * c, -2.0f * b * c, 1.0f - 2.0f * c * c, 0.0f,
            -2.0f * a * d, -2.0f * b * d, -2.0f * c * d, 1.0f
        );

        DirectX::XMStoreFloat4x4(&m_reflectionMatrix, reflectMat);
    }

    DirectX::XMMATRIX PlanarReflectionSystem::GetReflectionMatrix() const {
        return DirectX::XMLoadFloat4x4(&m_reflectionMatrix);
    }

    DirectX::XMMATRIX PlanarReflectionSystem::ComputeReflectionViewMatrix(const DirectX::XMMATRIX& mainView) const {
        // メインカメラのビュー行列の逆行列を取得（ワールド変換）
        DirectX::XMMATRIX invView = DirectX::XMMatrixInverse(nullptr, mainView);

        // 反射行列を適用
        DirectX::XMMATRIX reflectMat = GetReflectionMatrix();

        // 反射されたワールド変換を計算
        DirectX::XMMATRIX reflectedWorld = DirectX::XMMatrixMultiply(invView, reflectMat);

        // 反射されたビュー行列（逆行列）を返す
        return DirectX::XMMatrixInverse(nullptr, reflectedWorld);
    }

    Vector3 PlanarReflectionSystem::ReflectPosition(const Vector3& position) const {
        // P' = P - 2 * dot(P - P0, N) * N
        Vector3 toPoint = position - m_plane.m_point;
        float dist = Vector3::Dot(toPoint, m_plane.m_normal);
        return position - m_plane.m_normal * (2.0f * dist);
    }

    Vector3 PlanarReflectionSystem::ReflectDirection(const Vector3& direction) const {
        // D' = D - 2 * dot(D, N) * N
        float dot = Vector3::Dot(direction, m_plane.m_normal);
        return direction - m_plane.m_normal * (2.0f * dot);
    }

    DirectX::XMVECTOR PlanarReflectionSystem::ComputeClipPlane(const DirectX::XMMATRIX& viewMatrix) const {
        // ワールド空間のクリップ平面を取得
        DirectX::XMFLOAT4 worldPlane = GetWorldSpaceClipPlane();
        DirectX::XMVECTOR plane = DirectX::XMLoadFloat4(&worldPlane);

        // ビュー空間に変換
        // 平面 (a,b,c,d) をビュー空間に変換するには、ビュー行列の逆転置行列を使用
        DirectX::XMMATRIX invTransposeView = DirectX::XMMatrixTranspose(
            DirectX::XMMatrixInverse(nullptr, viewMatrix)
        );

        return DirectX::XMVector4Transform(plane, invTransposeView);
    }

    DirectX::XMFLOAT4 PlanarReflectionSystem::GetWorldSpaceClipPlane() const {
        // 平面方程式: ax + by + cz + d = 0
        // (a,b,c) = 法線, d = -dot(法線, 平面上の点)
        float d = -Vector3::Dot(m_plane.m_normal, m_plane.m_point);
        return DirectX::XMFLOAT4(
            m_plane.m_normal.x,
            m_plane.m_normal.y,
            m_plane.m_normal.z,
            d
        );
    }

    RenderTarget* PlanarReflectionSystem::GetReflectionTarget() {
        return m_reflectionRT.get();
    }

    ID3D11ShaderResourceView* PlanarReflectionSystem::GetReflectionSRV() const {
        if (m_reflectionRT) {
            return m_reflectionRT->GetShaderResourceView();
        }
        return nullptr;
    }

} // namespace Engine
