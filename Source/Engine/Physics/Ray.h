/// @file   Ray.h
/// @brief  レイ（光線）クラス - 原点と方向を持つ3D光線
#pragma once

#include "Engine/Math/Vector3.h"

namespace Engine {

    /// @brief 3D空間上の光線を表すクラス
    /// @note  原点(origin)と方向(direction)を持ち、任意の距離での点を計算可能
    class Ray final {
    public:
        /// @brief デフォルトコンストラクタ（原点(0,0,0)、方向(0,0,1)）
        Ray();

        /// @brief コンストラクタ
        /// @param origin レイの原点
        /// @param direction レイの方向（正規化される）
        Ray(const Vector3& origin, const Vector3& direction);

        /// @brief デストラクタ
        ~Ray() = default;

        //============================================================
        // Origin
        //============================================================

        /// @brief 原点を取得
        /// @return 原点への参照
        [[nodiscard]] const Vector3& GetOrigin() const;

        /// @brief 原点を設定
        /// @param origin 新しい原点
        void SetOrigin(const Vector3& origin);

        //============================================================
        // Direction
        //============================================================

        /// @brief 方向を取得
        /// @return 方向への参照（正規化済み）
        [[nodiscard]] const Vector3& GetDirection() const;

        /// @brief 方向を設定
        /// @param direction 新しい方向（内部で正規化される）
        void SetDirection(const Vector3& direction);

        //============================================================
        // Point Calculation
        //============================================================

        /// @brief 指定した距離での点を計算
        /// @param distance 原点からの距離
        /// @return レイ上の点（origin + direction * distance）
        [[nodiscard]] Vector3 GetPoint(float distance) const;

    private:
        Vector3 m_origin;       ///< レイの原点
        Vector3 m_direction;    ///< レイの方向（正規化済み）
    };

} // namespace Engine
