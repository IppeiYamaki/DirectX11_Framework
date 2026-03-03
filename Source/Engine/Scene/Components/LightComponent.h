/// @file   LightComponent.h
/// @brief  ライトをGameObjectにアタッチするためのComponent
#pragma once

#include "Engine/Scene/Component.h"
#include "Engine/Scene/Light.h"
#include "Engine/Math/Vector3.h"

namespace Engine {

    class LightSystem;

    /// @brief ライトをGameObjectにアタッチするためのComponent
    /// @note  Transform位置と同期し、LightSystemに登録される
    class LightComponent : public Component {
    public:
        /// @brief コンストラクタ
        /// @param lightSystem ライト管理システム（借用）
        explicit LightComponent(LightSystem* lightSystem);
        ~LightComponent() override;

        LightComponent(const LightComponent&) = delete;
        LightComponent& operator=(const LightComponent&) = delete;

        //============================================================
        // Lifecycle
        //============================================================

        /// @brief 初期化時に呼ばれる
        void OnAwake() override;

        /// @brief 毎フレーム更新（Transform位置とライト位置を同期）
        /// @param deltaTime フレーム経過時間
        void Update(float deltaTime) override;

        /// @brief 破棄時に呼ばれる
        void OnDestroy() override;

        //============================================================
        // Light Type
        //============================================================

        /// @brief  光源タイプを取得
        /// @return 光源タイプ
        [[nodiscard]] virtual LightType GetLightType() const = 0;

        //============================================================
        // Common Light Properties
        //============================================================

        /// @brief  光の色を設定
        /// @param  color RGB色（0〜1）
        virtual void SetColor(const Vector3& color) = 0;

        /// @brief  光の色を取得
        /// @return 光の色
        [[nodiscard]] virtual Vector3 GetColor() const = 0;

        /// @brief  光の強度を設定
        /// @param  intensity 強度（0以上）
        virtual void SetIntensity(float intensity) = 0;

        /// @brief  光の強度を取得
        /// @return 光の強度
        [[nodiscard]] virtual float GetIntensity() const = 0;

        /// @brief  環境光色を設定
        /// @param  ambient 環境光色（RGB）
        virtual void SetAmbient(const Vector3& ambient) = 0;

        /// @brief  環境光色を取得
        /// @return 環境光色
        [[nodiscard]] virtual Vector3 GetAmbient() const = 0;

        /// @brief  シャドウの有効/無効を設定
        /// @param  castShadow シャドウを生成するならtrue
        virtual void SetCastShadow(bool castShadow) = 0;

        /// @brief  シャドウを生成するか確認
        /// @return シャドウを生成するならtrue
        [[nodiscard]] virtual bool IsCastShadow() const = 0;

        /// @brief  内部Lightオブジェクトへのポインタを取得
        /// @return Light*
        [[nodiscard]] virtual Light* GetLightObject() = 0;

        /// @brief  内部Lightオブジェクトへのポインタを取得（const版）
        /// @return const Light*
        [[nodiscard]] virtual const Light* GetLightObject() const = 0;

    protected:
        /// @brief LightSystemにライトを登録
        virtual void RegisterLight() = 0;

        /// @brief LightSystemからライトを登録解除
        virtual void UnregisterLight() = 0;

        /// @brief Transform位置をライト位置に同期
        virtual void SyncPositionFromTransform() {}

        /// @brief Transform方向をライト方向に同期
        virtual void SyncDirectionFromTransform() {}

    protected:
        LightSystem* m_lightSystem = nullptr;  ///< ライト管理システム（借用）
        bool m_isRegistered = false;           ///< 登録済みフラグ
    };

    //============================================================
    // DirectionalLightComponent
    //============================================================

    /// @brief 方向性ライト（太陽光など）をGameObjectにアタッチ
    class DirectionalLightComponent final : public LightComponent {
    public:
        /// @brief コンストラクタ
        /// @param lightSystem ライト管理システム
        /// @param direction 光の方向（正規化される）
        explicit DirectionalLightComponent(LightSystem* lightSystem, const Vector3& direction = Vector3(0.0f, -1.0f, 0.0f));
        ~DirectionalLightComponent() override = default;

        //============================================================
        // Light Type
        //============================================================

        [[nodiscard]] LightType GetLightType() const override { return LightType::Directional; }

        //============================================================
        // Direction
        //============================================================

        /// @brief  光の方向を設定（正規化される）
        /// @param  direction 光の方向ベクトル
        void SetDirection(const Vector3& direction);

        /// @brief  光の方向を取得
        /// @return 正規化された光の方向
        [[nodiscard]] Vector3 GetDirection() const;

        //============================================================
        // Common Light Properties Implementation
        //============================================================

        void SetColor(const Vector3& color) override;
        [[nodiscard]] Vector3 GetColor() const override;
        void SetIntensity(float intensity) override;
        [[nodiscard]] float GetIntensity() const override;
        void SetAmbient(const Vector3& ambient) override;
        [[nodiscard]] Vector3 GetAmbient() const override;
        void SetCastShadow(bool castShadow) override;
        [[nodiscard]] bool IsCastShadow() const override;

        [[nodiscard]] Light* GetLightObject() override;
        [[nodiscard]] const Light* GetLightObject() const override;

    protected:
        void RegisterLight() override;
        void UnregisterLight() override;
        void SyncDirectionFromTransform() override;

    private:
        DirectionalLightObject* m_light = nullptr;  ///< 内部ライトオブジェクト
        Vector3 m_initialDirection{0.0f, -1.0f, 0.0f};  ///< 初期方向
    };

    //============================================================
    // PointLightComponent
    //============================================================

    /// @brief 点光源（街灯、炎など）をGameObjectにアタッチ
    class PointLightComponent final : public LightComponent {
    public:
        /// @brief コンストラクタ
        /// @param lightSystem ライト管理システム
        explicit PointLightComponent(LightSystem* lightSystem);
        ~PointLightComponent() override = default;

        //============================================================
        // Light Type
        //============================================================

        [[nodiscard]] LightType GetLightType() const override { return LightType::Point; }

        //============================================================
        // Range / Attenuation
        //============================================================

        /// @brief  光の到達距離を設定
        /// @param  range 到達距離
        void SetRange(float range);

        /// @brief  光の到達距離を取得
        /// @return 到達距離
        [[nodiscard]] float GetRange() const;

        /// @brief  減衰パラメータを設定（定数、線形、二次）
        /// @param  constant 定数項
        /// @param  linear 線形項
        /// @param  quadratic 二次項
        void SetAttenuation(float constant, float linear, float quadratic);

        /// @brief 減衰パラメータを取得
        /// @param outConstant 定数項
        /// @param outLinear 線形項
        /// @param outQuadratic 二次項
        void GetAttenuation(float& outConstant, float& outLinear, float& outQuadratic) const;

        //============================================================
        // Common Light Properties Implementation
        //============================================================

        void SetColor(const Vector3& color) override;
        [[nodiscard]] Vector3 GetColor() const override;
        void SetIntensity(float intensity) override;
        [[nodiscard]] float GetIntensity() const override;
        void SetAmbient(const Vector3& ambient) override;
        [[nodiscard]] Vector3 GetAmbient() const override;
        void SetCastShadow(bool castShadow) override;
        [[nodiscard]] bool IsCastShadow() const override;

        [[nodiscard]] Light* GetLightObject() override;
        [[nodiscard]] const Light* GetLightObject() const override;

    protected:
        void RegisterLight() override;
        void UnregisterLight() override;
        void SyncPositionFromTransform() override;

    private:
        PointLightObject* m_light = nullptr;  ///< 内部ライトオブジェクト
    };

    //============================================================
    // SpotLightComponent
    //============================================================

    /// @brief スポットライト（懐中電灯、舞台照明など）をGameObjectにアタッチ
    class SpotLightComponent final : public LightComponent {
    public:
        /// @brief コンストラクタ
        /// @param lightSystem ライト管理システム
        /// @param direction 光の方向
        /// @param innerAngle 内側コーン角度（度）
        /// @param outerAngle 外側コーン角度（度）
        explicit SpotLightComponent(LightSystem* lightSystem,
            const Vector3& direction = Vector3(0.0f, -1.0f, 0.0f),
            float innerAngle = 30.0f,
            float outerAngle = 45.0f);
        ~SpotLightComponent() override = default;

        //============================================================
        // Light Type
        //============================================================

        [[nodiscard]] LightType GetLightType() const override { return LightType::Spot; }

        //============================================================
        // Direction
        //============================================================

        /// @brief  光の方向を設定（正規化される）
        /// @param  direction 光の方向ベクトル
        void SetDirection(const Vector3& direction);

        /// @brief  光の方向を取得
        /// @return 正規化された光の方向
        [[nodiscard]] Vector3 GetDirection() const;

        //============================================================
        // Range / Attenuation
        //============================================================

        /// @brief  光の到達距離を設定
        /// @param  range 到達距離
        void SetRange(float range);

        /// @brief  光の到達距離を取得
        /// @return 到達距離
        [[nodiscard]] float GetRange() const;

        /// @brief  減衰パラメータを設定（定数、線形、二次）
        /// @param  constant 定数項
        /// @param  linear 線形項
        /// @param  quadratic 二次項
        void SetAttenuation(float constant, float linear, float quadratic);

        /// @brief 減衰パラメータを取得
        /// @param outConstant 定数項
        /// @param outLinear 線形項
        /// @param outQuadratic 二次項
        void GetAttenuation(float& outConstant, float& outLinear, float& outQuadratic) const;

        //============================================================
        // Cone Angles
        //============================================================

        /// @brief  コーン角度を設定（度数）
        /// @param  innerAngle 内側角度（フル輝度の範囲）
        /// @param  outerAngle 外側角度（フェードアウト終了位置）
        void SetConeAngles(float innerAngle, float outerAngle);

        /// @brief  コーン角度を取得（度数）
        /// @param  outInnerAngle 内側角度
        /// @param  outOuterAngle 外側角度
        void GetConeAngles(float& outInnerAngle, float& outOuterAngle) const;

        //============================================================
        // Common Light Properties Implementation
        //============================================================

        void SetColor(const Vector3& color) override;
        [[nodiscard]] Vector3 GetColor() const override;
        void SetIntensity(float intensity) override;
        [[nodiscard]] float GetIntensity() const override;
        void SetAmbient(const Vector3& ambient) override;
        [[nodiscard]] Vector3 GetAmbient() const override;
        void SetCastShadow(bool castShadow) override;
        [[nodiscard]] bool IsCastShadow() const override;

        [[nodiscard]] Light* GetLightObject() override;
        [[nodiscard]] const Light* GetLightObject() const override;

    protected:
        void RegisterLight() override;
        void UnregisterLight() override;
        void SyncPositionFromTransform() override;
        void SyncDirectionFromTransform() override;

    private:
        SpotLightObject* m_light = nullptr;  ///< 内部ライトオブジェクト
    };

} // namespace Engine
