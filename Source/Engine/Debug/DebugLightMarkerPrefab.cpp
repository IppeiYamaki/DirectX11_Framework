/// @file   DebugLightMarkerPrefab.cpp
/// @brief  デバッグシステム用ライトマーカーPrefab実装
#include "DebugLightMarkerPrefab.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/MeshRenderer.h"
#include "Engine/Graphics/RenderSystem.h"

namespace Engine {

    GameObject* DebugLightMarkerPrefab::Spawn(SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_scene || !ctx.m_renderSystem || !ctx.m_device) {
            Logger::Error("DebugLightMarkerPrefab::Spawn failed: ctx invalid.");
            return nullptr;
        }

        // GameObjectを作成（デバッグマーカー用タグ付き）
        auto* obj = ctx.m_scene->CreateObject<GameObject>("DebugLightMarker");
        if (!obj) return nullptr;

        // デバッグオブジェクト識別用タグを追加
        obj->AddTag("DebugMarker");

        // Transform 初期値
        if (auto* tr = obj->GetComponent<Transform>()) {
            tr->SetPosition(desc.m_position);
            tr->SetUniformScale(desc.m_size);
        }

        // MeshRenderer（球体メッシュ）
        auto* mr = obj->AddComponent<MeshRenderer>(ctx.m_device, ctx.m_renderSystem);
        mr->SetMeshType(MeshType::Sphere);

        // デバッグマーカーはDebugレイヤーを使用（常に最前面に描画）
        mr->SetRenderLayer(RenderLayer::Debug);

        // Note: マテリアルは設定しない（デフォルトの描画を使用）
        // 将来的にDebug用マテリアルを追加することも可能

        return obj;
    }

} // namespace Engine
