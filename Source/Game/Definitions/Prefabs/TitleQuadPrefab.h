#pragma once

#include "Engine/Math/Vector3.h"

namespace Engine { 
	struct SceneContext;
    class GameObject;
}

namespace Game {


    constexpr float kTitleQuadWidth = 20.0f;    // 幅
    constexpr float kTitleQuadHeight = 10.0f;   // 高さ

    /**
     * @brief タイトル画像表示用Quad（3D板ポリゴン）
     * - Canvas UIではなく、3D空間上に配置されるQuad
     * - Unlit（ライト影響なし）でTitle.pngをそのまま表示
     * - αブレンド対応（透過PNG対応）
     * - カメラ正面を向くように配置（固定カメラなので固定回転で対応）
     * 
     * @note 後で Canvas UI 上に Scene遷移ボタンを重ねて実装する予定
     *       ボタン実装は今回は不要
     */
    class TitleQuadPrefab final {
    public:
        struct SpawnDesc final {
            /// @brief Quadの位置（ワールド座標）
            Engine::Vector3 m_position{ 0, 0, 0 };

            /// @brief Quadの幅（調整用：将来調整しやすいように定数化）
            /// @note デフォルト値は仮値。必要に応じて調整
            float m_width = 8.0f;

            /// @brief Quadの高さ（調整用：将来調整しやすいように定数化）
            /// @note デフォルト値は仮値。必要に応じて調整
            float m_height = 4.5f;

            SpawnDesc() = default;
            SpawnDesc(const Engine::Vector3& position, float width = 8.0f, float height = 4.5f)
                : m_position(position), m_width(width), m_height(height) {
            }
        };

        static Engine::GameObject* Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc);
    };

} // namespace Game
