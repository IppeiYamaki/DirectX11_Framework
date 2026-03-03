/// @file   HeightfieldColliderComponent.h
/// @brief  ハイトフィールドコライダーコンポーネント（地形用）
#pragma once

#include "Engine/Physics/ColliderComponent.h"
#include <functional>

namespace Engine {

    /// @brief ハイトフィールドコライダーコンポーネント
    /// @note  地形（Field）との衝突判定に使用。高さマップから法線を計算できる
    class HeightfieldColliderComponent : public ColliderComponent {
    public:
        /// @brief 高さ取得関数の型（x, z座標から高さを返す）
        using HeightFunction = std::function<float(float x, float z)>;

        HeightfieldColliderComponent();
        ~HeightfieldColliderComponent() override = default;

        //============================================================
        // 形状設定
        //============================================================

        /// @brief 高さ取得関数を設定
        /// @param func 高さ取得関数
        void SetHeightFunction(const HeightFunction& func);

        /// @brief 範囲を設定
        /// @param minX X最小値
        /// @param maxX X最大値
        /// @param minZ Z最小値
        /// @param maxZ Z最大値
        void SetBounds(float minX, float maxX, float minZ, float maxZ);

        /// @brief 指定位置の高さを取得
        /// @param x X座標
        /// @param z Z座標
        /// @return 高さ（範囲外の場合はデフォルト高さ）
        [[nodiscard]] float GetHeightAt(float x, float z) const;

        /// @brief 指定位置の法線を取得
        /// @param x X座標
        /// @param z Z座標
        /// @return 地面法線
        [[nodiscard]] Vector3 GetNormalAt(float x, float z) const;

        /// @brief デフォルト高さを設定（範囲外用）
        /// @param height デフォルト高さ
        void SetDefaultHeight(float height);

        //============================================================
        // 衝突判定用ヘルパー
        //============================================================

        /// @brief 球とハイトフィールドの衝突判定
        /// @param sphereCenter 球の中心
        /// @param sphereRadius 球の半径
        /// @param outManifold 衝突結果出力
        /// @return 衝突していればtrue
        bool CheckSphereCollision(const Vector3& sphereCenter, float sphereRadius, CollisionManifold& outManifold);

        /// @brief AABBとハイトフィールドの衝突判定
        /// @param aabbMin AABBの最小点
        /// @param aabbMax AABBの最大点
        /// @param outManifold 衝突結果出力
        /// @return 衝突していればtrue
        bool CheckAABBCollision(const Vector3& aabbMin, const Vector3& aabbMax, CollisionManifold& outManifold);

        /// @brief カプセルとハイトフィールドの衝突判定
        /// @param bottom カプセル下端中心
        /// @param top カプセル上端中心
        /// @param radius カプセル半径
        /// @param outManifold 衝突結果出力
        /// @return 衝突していればtrue
        bool CheckCapsuleCollision(const Vector3& bottom, const Vector3& top, float radius, CollisionManifold& outManifold);

        //============================================================
        // オーバーライド
        //============================================================

        /// @brief 衝突検出
        /// @param other 相手コライダー
        /// @param outManifold 衝突結果出力
        /// @return 衝突していればtrue
        bool CheckCollision(ColliderComponent& other, CollisionManifold& outManifold) override;

    private:
        /// @brief 指定位置が範囲内かチェック
        /// @param x X座標
        /// @param z Z座標
        /// @return 範囲内ならtrue
        [[nodiscard]] bool IsInBounds(float x, float z) const;

    private:
        HeightFunction m_heightFunc;    ///< 高さ取得関数
        float m_minX = -1000.0f;        ///< X最小値
        float m_maxX = 1000.0f;         ///< X最大値
        float m_minZ = -1000.0f;        ///< Z最小値
        float m_maxZ = 1000.0f;         ///< Z最大値
        float m_defaultHeight = 0.0f;   ///< 範囲外のデフォルト高さ
        float m_sampleDelta = 0.1f;     ///< 法線計算用サンプル間隔
    };

} // namespace Engine
