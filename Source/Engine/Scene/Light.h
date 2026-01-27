/// @file   Light.h
/// @brief  光源の基底クラスと派生クラス
#pragma once

#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"

namespace Engine {

    /// @brief 光源タイプ
    enum class LightType : int {
        Directional = 0, ///< 方向性ライト
        Point = 1,       ///< 点光源
        Spot = 2         ///< スポットライト
    };

    /// @brief 光源の基底クラス
    /// @note  全ての光源タイプに共通のプロパティを持つ
    class Light {
    public:
        Light() = default;
        virtual ~Light() = default;

        Light(const Light&) = delete;
        Light& operator=(const Light&) = delete;

        //============================================================
        // Light Type
        //============================================================

        /// @brief  光源タイプを取得
        /// @return 光源タイプ
        [[nodiscard]] virtual LightType GetLightType() const = 0;

        //============================================================
        // Color
        //============================================================

        /// @brief  光の色を設定
        /// @param  color RGB色（0〜1）
        void SetColor(const Vector3& color);

        /// @brief  光の色を取得
        /// @return 光の色
        [[nodiscard]] Vector3 GetColor() const;

        //============================================================
        // Intensity
        //============================================================

        /// @brief  光の強度を設定
        /// @param  intensity 強度（0以上）
        void SetIntensity(float intensity);

        /// @brief  光の強度を取得
        /// @return 光の強度
        [[nodiscard]] float GetIntensity() const;

        //============================================================
        // Enable / Disable
        //============================================================

        /// @brief ライトを有効化
        void Enable();

        /// @brief ライトを無効化
        void Disable();

        /// @brief  ライトが有効か確認
        /// @return 有効ならtrue
        [[nodiscard]] bool IsEnabled() const;

        //============================================================
        // Shadow
        //============================================================

        /// @brief  シャドウの有効/無効を設定
        /// @param  castShadow シャドウを生成するならtrue
        void SetCastShadow(bool castShadow);

        /// @brief  シャドウを生成するか確認
        /// @return シャドウを生成するならtrue
        [[nodiscard]] bool IsCastShadow() const;

        //============================================================
        // Ambient
        //============================================================

        /// @brief  環境光色を設定
        /// @param  ambient 環境光色（RGB）
        void SetAmbient(const Vector3& ambient);

        /// @brief  環境光色を取得
        /// @return 環境光色
        [[nodiscard]] Vector3 GetAmbient() const;

    protected:
        Vector3 m_color{1.0f, 1.0f, 1.0f};    ///< ライトカラー（RGB）
        float m_intensity = 1.0f;              ///< 光の強さ
        bool m_isEnabled = true;               ///< 有効フラグ
        bool m_castShadow = false;             ///< シャドウ生成フラグ
        Vector3 m_ambient{0.1f, 0.1f, 0.1f};   ///< 環境光色
    };

    /// @brief 方向性ライト（太陽光のような無限遠からの光）
    class DirectionalLightObject : public Light {
    public:
        DirectionalLightObject() = default;
        ~DirectionalLightObject() override = default;

        /// @brief  光源タイプを取得
        /// @return Directional
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

    private:
        Vector3 m_direction{0.0f, -1.0f, 0.0f}; ///< 光の方向（正規化済み）
    };

    /// @brief 点光源（全方向に放射される光）
    class PointLightObject : public Light {
    public:
        PointLightObject() = default;
        ~PointLightObject() override = default;

        /// @brief  光源タイプを取得
        /// @return Point
        [[nodiscard]] LightType GetLightType() const override { return LightType::Point; }

        //============================================================
        // Position
        //============================================================

        /// @brief  光の位置を設定
        /// @param  position ワールド空間での位置
        void SetPosition(const Vector3& position);

        /// @brief  光の位置を取得
        /// @return ワールド空間での位置
        [[nodiscard]] Vector3 GetPosition() const;

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

    protected:
        Vector3 m_position{0.0f, 0.0f, 0.0f}; ///< 光の位置
        float m_range = 10.0f;                 ///< 到達距離
        float m_constantAttenuation = 1.0f;    ///< 定数減衰
        float m_linearAttenuation = 0.09f;     ///< 線形減衰
        float m_quadraticAttenuation = 0.032f; ///< 二次減衰
    };

    /// @brief スポットライト（円錐状に放射される光）
    class SpotLightObject : public PointLightObject {
    public:
        SpotLightObject() = default;
        ~SpotLightObject() override = default;

        /// @brief  光源タイプを取得
        /// @return Spot
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

        /// @brief  内側コーンのコサイン値を取得
        /// @return cos(innerAngle)
        [[nodiscard]] float GetInnerCosAngle() const;

        /// @brief  外側コーンのコサイン値を取得
        /// @return cos(outerAngle)
        [[nodiscard]] float GetOuterCosAngle() const;

    private:
        Vector3 m_direction{0.0f, -1.0f, 0.0f}; ///< 光の方向（正規化済み）
        float m_innerAngle = 30.0f;              ///< 内側コーン角度（度）
        float m_outerAngle = 45.0f;              ///< 外側コーン角度（度）
        float m_innerCosAngle = 0.866f;          ///< cos(innerAngle)キャッシュ
        float m_outerCosAngle = 0.707f;          ///< cos(outerAngle)キャッシュ
    };

} // namespace Engine
