/// @file   RayManager.h
/// @brief  レイの生成と管理を担当するクラス
#pragma once

#include <vector>
#include <cstddef>

#include "Engine/Physics/Ray.h"
#include "Engine/Math/Vector3.h"

namespace Engine {

    /// @brief 複数のレイを一括管理するマネージャークラス
    /// @note  任意の方向（カメラ方向、プレイヤー方向など）のレイを動的に追加・管理可能
    class RayManager final {
    public:
        RayManager() = default;
        ~RayManager() = default;

        RayManager(const RayManager&) = delete;
        RayManager& operator=(const RayManager&) = delete;

        //============================================================
        // Ray Management
        //============================================================

        /// @brief 新しいレイを追加
        /// @param origin レイの起点
        /// @param direction レイの方向
        /// @return 追加されたレイのインデックス
        std::size_t AddRay(const Vector3& origin, const Vector3& direction);

        /// @brief レイオブジェクトを直接追加
        /// @param ray 追加するレイ
        /// @return 追加されたレイのインデックス
        std::size_t AddRay(const Ray& ray);

        /// @brief 指定インデックスのレイを取得
        /// @param index レイのインデックス
        /// @return レイへのポインタ（範囲外の場合nullptr）
        [[nodiscard]] Ray* GetRay(std::size_t index);

        /// @brief 指定インデックスのレイを取得（const版）
        /// @param index レイのインデックス
        /// @return レイへのconstポインタ（範囲外の場合nullptr）
        [[nodiscard]] const Ray* GetRay(std::size_t index) const;

        /// @brief 指定インデックスのレイを削除
        /// @param index 削除するレイのインデックス
        /// @return 削除に成功したらtrue
        bool RemoveRay(std::size_t index);

        /// @brief 全てのレイを削除
        void ClearRays();

        /// @brief  登録されているレイの数を取得
        /// @return レイの数
        [[nodiscard]] std::size_t GetRayCount() const;

        //============================================================
        // Ray Operations
        //============================================================

        /// @brief 指定インデックスのレイの起点を更新
        /// @param index レイのインデックス
        /// @param origin 新しい起点
        /// @return 更新に成功したらtrue
        bool UpdateRayOrigin(std::size_t index, const Vector3& origin);

        /// @brief 指定インデックスのレイの方向を更新
        /// @param index レイのインデックス
        /// @param direction 新しい方向
        /// @return 更新に成功したらtrue
        bool UpdateRayDirection(std::size_t index, const Vector3& direction);

    private:
        std::vector<Ray> m_rays;    ///< 登録されたレイのリスト
    };

} // namespace Engine
