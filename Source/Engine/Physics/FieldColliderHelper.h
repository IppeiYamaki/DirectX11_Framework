/// @file   FieldColliderHelper.h
/// @brief  Field地形用のコライダーヘルパーコンポーネント
/// @note   MeshRenderer(Field)と一緒に使用して、地形の高さ取得機能を提供する
#pragma once

#include <memory>

#include "Engine/Scene/Component.h"
#include "Engine/Physics/HeightfieldColliderComponent.h"
#include <cstdint>

namespace Engine {

    class PerlinNoise;

    /// @brief Field地形用コライダーヘルパー
    /// @note  MeshCreateDesc::Fieldの設定を受け取り、HeightfieldColliderの高さ関数を提供する
    class FieldColliderHelper : public Component {
    public:
        FieldColliderHelper();
        ~FieldColliderHelper() override;

        FieldColliderHelper(const FieldColliderHelper&) = delete;
        FieldColliderHelper& operator=(const FieldColliderHelper&) = delete;

        //============================================================
        // Lifecycle
        //============================================================

        /// @brief Awake時に呼ばれる
        void OnAwake() override;

        //============================================================
        // 設定
        //============================================================

        /// @brief フィールドパラメータを設定
        /// @param width フィールド幅
        /// @param depth フィールド奥行き
        /// @param amplitude 高さスケール
        /// @param frequency ノイズ周波数
        /// @param octaves オクターブ数
        /// @param seed シード値
        void SetFieldParams(float width, float depth,
                           float amplitude, float frequency,
                           int octaves, std::uint32_t seed);

        /// @brief 指定位置の高さを取得
        /// @param x X座標（ワールド座標）
        /// @param z Z座標（ワールド座標）
        /// @return 高さ（Y座標）
        [[nodiscard]] float GetHeightAt(float x, float z) const;

        /// @brief 指定位置の法線を取得
        /// @param x X座標（ワールド座標）
        /// @param z Z座標（ワールド座標）
        /// @return 地面法線
        [[nodiscard]] Vector3 GetNormalAt(float x, float z) const;

        //============================================================
        // HeightfieldCollider連携
        //============================================================

        /// @brief HeightfieldColliderComponentを自動設定
        /// @param collider 設定先のコライダー
        void SetupHeightfieldCollider(HeightfieldColliderComponent* collider);

    private:
        /// @brief フラクタルノイズで高さを計算
        /// @param x ノイズ空間のX座標
        /// @param z ノイズ空間のZ座標
        /// @return 正規化された高さ値
        [[nodiscard]] float SampleFractalNoise(float x, float z) const;

    private:
        // フィールドパラメータ
        float m_width = 100.0f;
        float m_depth = 100.0f;
        float m_amplitude = 10.0f;
        float m_frequency = 0.05f;
        int m_octaves = 4;
        std::uint32_t m_seed = 0;

        // Perlin Noise生成器
        std::unique_ptr<PerlinNoise> m_perlinNoise;

        // フィールドのワールド位置オフセット
        Vector3 m_worldOffset;
    };

} // namespace Engine
