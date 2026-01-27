/// @file   LightSystem.h
/// @brief  光源の生成・管理を統括するクラス
#pragma once

#include <memory>
#include <vector>

#include "Engine/Math/Vector3.h"
#include "Engine/Scene/Light.h"

namespace Engine {

    class RenderSystem;
    class DirectionalLightObject;
    class PointLightObject;
    class SpotLightObject;
    class Scene;

    /// @brief 光源の生成・管理を統括するシステム
    /// @note  複数光源の管理とシェーダーへのデータ受け渡しを担当
    class LightSystem final {
    public:
        LightSystem() = default;
        ~LightSystem();

        LightSystem(const LightSystem&) = delete;
        LightSystem& operator=(const LightSystem&) = delete;

        //============================================================
        // Lifecycle
        //============================================================

        /// @brief  初期化
        /// @param  renderSystem RenderSystem参照
        /// @return 成功ならtrue
        [[nodiscard]] bool Initialize(RenderSystem* renderSystem);

        /// @brief 終了処理
        void Finalize();

        //============================================================
        // Light Management - Directional
        //============================================================

        /// @brief  方向性ライトを追加
        /// @param  direction 光の方向ベクトル
        /// @return 生成されたライトへのポインタ
        DirectionalLightObject* AddDirectionalLight(const Vector3& direction);

        //============================================================
        // Light Management - Point
        //============================================================

        /// @brief  点光源を追加
        /// @param  position 光の位置
        /// @return 生成されたライトへのポインタ
        PointLightObject* AddPointLight(const Vector3& position);

        //============================================================
        // Light Management - Spot
        //============================================================

        /// @brief  スポットライトを追加
        /// @param  position 光の位置
        /// @param  direction 光の方向
        /// @param  innerAngle 内側コーン角度（度）
        /// @param  outerAngle 外側コーン角度（度）
        /// @return 生成されたライトへのポインタ
        SpotLightObject* AddSpotLight(const Vector3& position, const Vector3& direction, float innerAngle, float outerAngle);

        //============================================================
        // Light Management - Common
        //============================================================

        /// @brief ライトを削除
        /// @param light 削除するライト
        void RemoveLight(Light* light);

        /// @brief 全てのライトを削除
        void ClearLights();

        /// @brief  ライト数を取得
        /// @return ライト数
        [[nodiscard]] std::size_t GetLightCount() const;

        /// @brief  方向性ライト数を取得
        /// @return 方向性ライト数
        [[nodiscard]] std::size_t GetDirectionalLightCount() const;

        /// @brief  点光源数を取得
        /// @return 点光源数
        [[nodiscard]] std::size_t GetPointLightCount() const;

        /// @brief  スポットライト数を取得
        /// @return スポットライト数
        [[nodiscard]] std::size_t GetSpotLightCount() const;

        //============================================================
        // Frame
        //============================================================

        /// @brief ライト情報を更新
        /// @param deltaTime フレーム経過時間
        void Update(float deltaTime);

        /// @brief ライト情報をRenderSystemに適用
        void ApplyToRenderSystem();

        //============================================================
        // Query
        //============================================================

        /// @brief  全ての方向性ライトを取得
        /// @return 方向性ライトのベクター
        [[nodiscard]] std::vector<DirectionalLightObject*> GetDirectionalLights() const;

        /// @brief  全ての点光源を取得
        /// @return 点光源のベクター
        [[nodiscard]] std::vector<PointLightObject*> GetPointLights() const;

        /// @brief  全てのスポットライトを取得
        /// @return スポットライトのベクター
        [[nodiscard]] std::vector<SpotLightObject*> GetSpotLights() const;

        /// @brief  初期化済みか確認
        /// @return 初期化済みならtrue
        [[nodiscard]] bool IsInitialized() const;

    private:
        RenderSystem* m_renderSystem = nullptr;            ///< RenderSystem参照（借用）
        std::vector<std::unique_ptr<Light>> m_lights;      ///< ライトのリスト
        bool m_isInitialized = false;                      ///< 初期化済みフラグ
    };

} // namespace Engine
