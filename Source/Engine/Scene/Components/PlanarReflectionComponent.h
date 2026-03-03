#pragma once

#include "Engine/Scene/Component.h"
#include "Engine/Math/Vector3.h"

#include <memory>

namespace Engine {

    class PlanarReflectionSystem;
    class GraphicsDevice;

    /// @brief 平面反射コンポーネント
    /// @note  反射平面の位置・法線をTransformから取得し、PlanarReflectionSystemに設定する
    ///        このコンポーネントはGameObjectに付けることで、反射システムがGameObjectとして管理される
    class PlanarReflectionComponent : public Component {
    public:
        /// @brief コンストラクタ
        /// @param reflectionSystem 平面反射システム（借用）
        explicit PlanarReflectionComponent(PlanarReflectionSystem* reflectionSystem);
        ~PlanarReflectionComponent() override = default;

        /// @brief 初期化
        void OnStart() override;

        /// @brief 更新（毎フレーム反射平面を更新）
        /// @param deltaTime フレーム経過時間
        void Update(float deltaTime) override;

        //============================================================
        // Settings
        //============================================================

        /// @brief 反射平面の法線を設定（ローカル座標）
        /// @param normal 法線ベクトル
        void SetPlaneNormal(const Vector3& normal) { m_planeNormal = normal.Normalized(); }

        /// @brief 反射平面のオフセットを設定（Transformからの相対位置）
        /// @param offset オフセット
        void SetPlaneOffset(const Vector3& offset) { m_planeOffset = offset; }

        /// @brief 反射システムを取得
        /// @return 反射システムへのポインタ
        PlanarReflectionSystem* GetReflectionSystem() const { return m_reflectionSystem; }

    private:
        PlanarReflectionSystem* m_reflectionSystem = nullptr;

        /// @brief 反射平面の法線（ローカル座標、デフォルトは上向き）
        Vector3 m_planeNormal{ 0.0f, 1.0f, 0.0f };

        /// @brief 反射平面のオフセット（Transformからの相対位置）
        Vector3 m_planeOffset{ 0.0f, 0.0f, 0.0f };
    };

} // namespace Engine
