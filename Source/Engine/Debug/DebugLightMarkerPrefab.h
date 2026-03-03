/// @file   DebugLightMarkerPrefab.h
/// @brief  デバッグシステム用ライトマーカーPrefab（球体メッシュ）
#pragma once

#include "Engine/Math/Vector3.h"
#include "Engine/Scene/SceneContext.h"

namespace Engine {

    class GameObject;

    /// @brief デバッグシステム用ライトマーカーPrefab
    /// @note  デバッグモード有効時にライト位置に表示する球体
    class DebugLightMarkerPrefab final {
    public:
        struct SpawnDesc final {
            Vector3 m_position{ 0.0f, 0.0f, 0.0f };   ///< マーカー位置
            Vector3 m_color{ 1.0f, 1.0f, 0.0f };      ///< マーカー色（ライト色）
            float m_size = 0.3f;                       ///< マーカーサイズ（半径）

            SpawnDesc() = default;

            SpawnDesc(const Vector3& position,
                      const Vector3& color = Vector3(1.0f, 1.0f, 0.0f),
                      float size = 0.3f)
                : m_position(position)
                , m_color(color)
                , m_size(size) {
            }
        };

        /// @brief デバッグライトマーカーを生成
        /// @param ctx SceneContext
        /// @param desc 生成パラメータ
        /// @return 生成されたGameObject
        static GameObject* Spawn(SceneContext& ctx, const SpawnDesc& desc);
    };

} // namespace Engine
