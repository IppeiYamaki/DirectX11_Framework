#pragma once

#include <DirectXMath.h>

#include "Engine/Scene/Component.h"

namespace Engine {

    class RenderSystem;

    /// @brief  メインカメラ（View/ProjectionをRenderSystemへ通知）
    /// @note   Transformの逆行列をViewとして用い、LateUpdateで反映する
    class Camera final : public Component {
    public:
        /// @brief  コンストラクタ
        /// @param  renderSystem RenderSystem参照
        explicit Camera(RenderSystem* renderSystem);
        ~Camera() override = default;

        /// @brief Start時に呼ばれる
        void OnStart() override;
        /// @brief LateUpdate時に呼ばれる
        /// @param deltaTime フレーム経過時間
        void LateUpdate(float deltaTime) override;

        /// @brief  メインカメラ設定を切り替える
        /// @param  isMain メインならtrue
        void SetMain(bool isMain);
        /// @brief  メインカメラか確認
        /// @return メインならtrue
        bool IsMain() const;

        /// @brief  透視投影パラメータを設定
        /// @param  fovYRadians 垂直方向FOV(ラジアン)
        /// @param  aspect アスペクト比
        /// @param  nearZ 近クリップ
        /// @param  farZ 遠クリップ
        void SetPerspective(float fovYRadians, float aspect, float nearZ, float farZ);

    private:
        /// @brief RenderSystemへカメラ定数を適用
        void ApplyToRenderSystem();

    private:
        RenderSystem* m_renderSystem = nullptr; ///< RenderSystem（借用）
        bool m_isMain = false;                  ///< メインカメラフラグ

        float m_fovY = DirectX::XM_PIDIV4; ///< 垂直FOV(ラジアン)
        float m_aspect = 16.0f / 9.0f;     ///< アスペクト比
        float m_nearZ = 0.1f;              ///< 近クリップ
        float m_farZ = 1000.0f;            ///< 遠クリップ
    };

} // namespace Engine
