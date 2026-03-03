#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

#include "Engine/Graphics/RenderTarget.h"
#include "Engine/Math/Vector3.h"

namespace Engine {

    class GraphicsDevice;
    class RenderSystem;
    class World;

    /// @brief 反射平面の定義
    struct ReflectionPlane final {
        Vector3 m_point{ 0.0f, 0.0f, 0.0f };    ///< 平面上の点
        Vector3 m_normal{ 0.0f, 1.0f, 0.0f };   ///< 平面の法線（正規化済み）

        ReflectionPlane() = default;
        ReflectionPlane(const Vector3& point, const Vector3& normal)
            : m_point(point), m_normal(normal.Normalized()) {}
    };

    /// @brief 平面反射システム
    /// @note  反射用カメラの生成、反射RTの管理、クリップ平面の処理を担当
    class PlanarReflectionSystem final {
    public:
        PlanarReflectionSystem() = default;
        ~PlanarReflectionSystem();

        PlanarReflectionSystem(const PlanarReflectionSystem&) = delete;
        PlanarReflectionSystem& operator=(const PlanarReflectionSystem&) = delete;

        //============================================================
        // Lifecycle
        //============================================================

        /// @brief  初期化
        /// @param  device グラフィックスデバイス
        /// @param  width 反射RT幅
        /// @param  height 反射RT高さ
        /// @return 成功ならtrue
        bool Initialize(GraphicsDevice* device, std::uint32_t width = 1024, std::uint32_t height = 1024);

        /// @brief 終了処理
        void Finalize();

        /// @brief  初期化済みか確認
        /// @return 初期化済みならtrue
        bool IsInitialized() const;

        //============================================================
        // Reflection Plane
        //============================================================

        /// @brief  反射平面を設定
        /// @param  plane 反射平面
        void SetReflectionPlane(const ReflectionPlane& plane);

        /// @brief  反射平面を取得
        /// @return 反射平面
        const ReflectionPlane& GetReflectionPlane() const;

        //============================================================
        // Camera Transform
        //============================================================

        /// @brief  メインカメラから反射カメラのビュー行列を計算
        /// @param  mainView メインカメラのビュー行列
        /// @return 反射カメラのビュー行列
        DirectX::XMMATRIX ComputeReflectionViewMatrix(const DirectX::XMMATRIX& mainView) const;

        /// @brief  カメラ位置を反射平面で反射
        /// @param  position 元の位置
        /// @return 反射後の位置
        Vector3 ReflectPosition(const Vector3& position) const;

        /// @brief  方向ベクトルを反射平面で反射
        /// @param  direction 元の方向
        /// @return 反射後の方向
        Vector3 ReflectDirection(const Vector3& direction) const;

        //============================================================
        // Clip Plane
        //============================================================

        /// @brief  クリップ平面を計算（ビュー空間）
        /// @param  viewMatrix 現在のビュー行列
        /// @return クリップ平面（ax + by + cz + d = 0形式のXMVECTOR）
        DirectX::XMVECTOR ComputeClipPlane(const DirectX::XMMATRIX& viewMatrix) const;

        /// @brief  ワールド空間でのクリップ平面を取得
        /// @return クリップ平面（XMFLOAT4形式：x,y,z=法線、w=距離）
        DirectX::XMFLOAT4 GetWorldSpaceClipPlane() const;

        //============================================================
        // Render Target
        //============================================================

        /// @brief  反射レンダーターゲットを取得
        /// @return RenderTarget
        RenderTarget* GetReflectionTarget();

        /// @brief  反射テクスチャのSRVを取得
        /// @return SRV
        ID3D11ShaderResourceView* GetReflectionSRV() const;

        //============================================================
        // Reflection Matrix
        //============================================================

        /// @brief  反射行列を取得
        /// @return 反射行列
        DirectX::XMMATRIX GetReflectionMatrix() const;

    private:
        /// @brief 反射行列を更新
        void UpdateReflectionMatrix();

    private:
        GraphicsDevice* m_device = nullptr;
        bool m_isInitialized = false;

        ReflectionPlane m_plane;                        ///< 反射平面
        DirectX::XMFLOAT4X4 m_reflectionMatrix{};       ///< 反射行列
        std::unique_ptr<RenderTarget> m_reflectionRT;   ///< 反射レンダーターゲット
    };

} // namespace Engine
