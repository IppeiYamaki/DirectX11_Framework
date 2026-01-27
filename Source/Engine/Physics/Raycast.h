/// @file   Raycast.h
/// @brief  レイキャスト機能 - 3Dオブジェクトとの衝突判定
#pragma once

#include "Engine/Math/Vector3.h"

namespace Engine {

    class Ray;
    class GameObject;
    class Scene;

    /// @brief レイキャストの結果を格納するクラス
    class RaycastResult final {
    public:
        RaycastResult();

        /// @brief ヒットしたか確認
        /// @return ヒットしていればtrue
        [[nodiscard]] bool HasHit() const;

        /// @brief ヒットしたオブジェクトを取得
        /// @return ヒットしたGameObject（ヒットなしの場合nullptr）
        [[nodiscard]] GameObject* GetHitObject() const;

        /// @brief ヒットポイントまでの距離を取得
        /// @return 距離
        [[nodiscard]] float GetDistance() const;

        /// @brief ヒットポイントを取得
        /// @return ワールド空間でのヒットポイント
        [[nodiscard]] const Vector3& GetHitPoint() const;

        /// @brief ヒットした面の法線を取得
        /// @return ヒット面の法線
        [[nodiscard]] const Vector3& GetNormal() const;

    private:
        friend class Physics;

        GameObject* m_hitObject = nullptr;  ///< ヒットしたオブジェクト
        float m_distance = 0.0f;            ///< ヒットポイントまでの距離
        Vector3 m_hitPoint;                 ///< ヒットポイント
        Vector3 m_normal;                   ///< ヒット面の法線
        bool m_hasHit = false;              ///< ヒットフラグ
    };

    /// @brief 物理演算・レイキャストを担当する静的クラス
    class Physics final {
    public:
        Physics() = delete;
        ~Physics() = delete;

        //============================================================
        // Raycast
        //============================================================

        /// @brief シーン内のGameObjectに対してレイキャストを行う
        /// @param ray レイ
        /// @param scene 対象シーン
        /// @param result 結果を格納するRaycastResult
        /// @param maxDistance 最大距離（デフォルト: 無限大）
        /// @return ヒットしたらtrue
        static bool Raycast(const Ray& ray, Scene* scene, RaycastResult& result, float maxDistance = 10000.0f);

        /// @brief シーン内のGameObjectに対してレイキャストを行う（結果なし版）
        /// @param ray レイ
        /// @param scene 対象シーン
        /// @param maxDistance 最大距離
        /// @return ヒットしたらtrue
        static bool Raycast(const Ray& ray, Scene* scene, float maxDistance = 10000.0f);

        //============================================================
        // Sphere Intersection（内部用）
        //============================================================

        /// @brief レイと球の交差判定
        /// @param ray レイ
        /// @param center 球の中心
        /// @param radius 球の半径
        /// @param outDistance 交差点までの距離（出力）
        /// @return 交差していればtrue
        static bool RaySphereIntersect(const Ray& ray, const Vector3& center, float radius, float& outDistance);

        //============================================================
        // AABB Intersection（内部用）
        //============================================================

        /// @brief レイとAABB（軸平行境界ボックス）の交差判定
        /// @param ray レイ
        /// @param min AABBの最小点
        /// @param max AABBの最大点
        /// @param outDistance 交差点までの距離（出力）
        /// @return 交差していればtrue
        static bool RayAABBIntersect(const Ray& ray, const Vector3& min, const Vector3& max, float& outDistance);
    };

} // namespace Engine
