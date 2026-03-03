#include "TitleQuadPrefab.h"

#include "Engine/Scene/SceneContext.h"

#include "Engine/Core/Logger.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/MeshRenderer.h"

#include "Engine/Graphics/MeshFactory.h"

#include "Engine/Materials/MaterialLibrary.h"
#include "Game/Definitions/Materials/TitleQuadMaterial.h"

namespace Game {

    Engine::GameObject* TitleQuadPrefab::Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_scene || !ctx.m_renderSystem || !ctx.m_device || !ctx.m_materials) {
            Engine::Logger::Error("TitleQuadPrefab::Spawn failed: ctx invalid.");
            return nullptr;
        }

        auto* obj = ctx.m_scene->CreateObject<Engine::GameObject>();
        if (!obj) {
            Engine::Logger::Error("TitleQuadPrefab::Spawn failed: could not create GameObject.");
            return nullptr;
        }

        // Transform設定
        // TitleSceneのカメラは固定なので、Quadも固定回転でカメラ正面を向ける
        // カメラがZ軸負方向を見ている場合、QuadはZ軸正方向を向く
        if (auto* tr = obj->GetComponent<Engine::Transform>()) {
            tr->SetPosition(desc.m_position);
            // Quadは既にZ軸正方向を向いているので回転は不要
            // （MeshPrimitives::CreateQuadの法線は(0,0,1)）
        }

        // MeshRenderer設定
        auto* mr = obj->AddComponent<Engine::MeshRenderer>(ctx.m_device, ctx.m_renderSystem);
        if (!mr) {
            Engine::Logger::Error("TitleQuadPrefab::Spawn failed: could not add MeshRenderer.");
            ctx.m_scene->DestroyObject(obj);
            return nullptr;
        }

        // Quad生成（指定サイズ）
        Engine::MeshCreateDesc meshDesc = Engine::MeshCreateDesc::Quad(desc.m_width, desc.m_height);
        mr->SetMeshDesc(meshDesc);

        // Transparent層で描画（αブレンド対応）
        // 水面より後に描画し、透明部分に水面が映るようにする
        mr->SetRenderLayer(Engine::RenderLayer::Transparent);
        mr->SetOrderInLayer(200);  // 水面(orderInLayer=100)より後に描画

        // αブレンド有効化（透過PNG対応）+ 深度書き込みオフ
        // 深度書き込みをオフにすることで、透明部分で水面が見えるようになる
        mr->SetRenderStateFlags(Engine::kRenderStateBlendAlpha | Engine::kRenderStateDepthWriteOff);

        // TitleQuadMaterial設定
        auto material = ctx.m_materials->GetOrCreate<Game::TitleQuadMaterial>();
        if (!material) {
            Engine::Logger::Warn("TitleQuadPrefab::Spawn: TitleQuadMaterial creation failed. Quad will not render.");
            // マテリアル作成失敗でもクラッシュしない（描画されないだけ）
        }
        mr->SetMaterial(material);

        Engine::Logger::Info("TitleQuadPrefab: Title Quad created at position (" +
            std::to_string(desc.m_position.x) + ", " +
            std::to_string(desc.m_position.y) + ", " +
            std::to_string(desc.m_position.z) + ")");

        return obj;
    }

} // namespace Game
