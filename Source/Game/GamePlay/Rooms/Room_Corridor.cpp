/// @file   Room_Corridor.cpp
/// @brief  デフォルト廊下クラス実装
#include "Game/Gameplay/Rooms/Room_Corridor.h"
#include "Game/Gameplay/CharacterInstance.h"

#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/StaticModelRenderer.h"
#include "Engine/Graphics/Model.h"
#include "Engine/Core/Logger.h"

namespace Game {

    Room_Corridor::Room_Corridor()
        : RoomInstance("Corridor", RoomType::Corridor)
    {
    }

    //============================================================
    // ビジュアル管理
    //============================================================

    bool Room_Corridor::CreateVisual(Engine::SceneContext& ctx, const Engine::Vector3& worldPosition) {
        // コンテキストの検証
        if (!ctx.m_scene || !ctx.m_device || !ctx.m_renderSystem || !ctx.m_assets) {
            Engine::Logger::Error("Room_Corridor::CreateVisual failed: ctx invalid.");
            return false;
        }

        // GameObjectを生成
        auto* obj = ctx.m_scene->CreateObject<Engine::GameObject>();
        if (!obj) {
            Engine::Logger::Error("Room_Corridor::CreateVisual failed: Failed to create GameObject.");
            return false;
        }

        obj->SetName("Corridor_Tile");

        // Transformを設定
        if (auto* tr = obj->GetComponent<Engine::Transform>()) {
            tr->SetPosition(worldPosition);
            tr->SetUniformScale(100.0f);
        }

        // StaticModelRendererを追加してモデルを読み込み
        auto* renderer = obj->AddComponent<Engine::StaticModelRenderer>(
            ctx.m_assets,
            ctx.m_device,
            ctx.m_renderSystem
        );

        if (!renderer) {
            Engine::Logger::Error("Room_Corridor::CreateVisual: Failed to add StaticModelRenderer");
            ctx.m_scene->DestroyObject(obj);
            return false;
        }

        // モデルファイルを読み込み
        bool loaded = renderer->LoadModel(L"Assets/Models/Environment/Tiles/Tile_Corridor.fbx");
        if (!loaded) {
            Engine::Logger::Error("Room_Corridor::CreateVisual: Failed to load Tile_Corridor.obj");
            ctx.m_scene->DestroyObject(obj);
            return false;
        }

        // モデルとマテリアルの検証
        auto* model = renderer->GetModel();
        if (model) {
            const auto& subsets = model->GetSubsets();
            Engine::Logger::Info("Corridor model loaded with " + std::to_string(subsets.size()) + " subsets");

            for (const auto& subset : subsets) {
                if (!subset.m_material || !subset.m_material->IsInitialized()) {
                    Engine::Logger::Warn("Corridor model has uninitialized material");
                }
            }
        } else {
            Engine::Logger::Error("Room_Corridor::CreateVisual: Model is null after loading");
            ctx.m_scene->DestroyObject(obj);
            return false;
        }

        m_visualObject = obj;
        return true;
    }

    void Room_Corridor::SetVisualObject(Engine::GameObject* visualObject) {
        m_visualObject = visualObject;
    }

    Engine::GameObject* Room_Corridor::GetVisualObject() const {
        return m_visualObject;
    }

    void Room_Corridor::UpdateVisualPosition(const Engine::Vector3& worldPosition) {
        if (m_visualObject) {
            if (auto* tr = m_visualObject->GetComponent<Engine::Transform>()) {
                tr->SetPosition(worldPosition);
            }
        }
    }

    //============================================================
    // RoomInstance オーバーライド
    //============================================================

    void Room_Corridor::OnCharacterEnter(CharacterInstance* /*character*/) {
        // 廊下は特別な効果なし
        // 将来的にはログ出力や通過カウントの記録などを追加可能
    }

    bool Room_Corridor::CanBeRemoved() const {
        // 廊下は常に削除可能
        return true;
    }

} // namespace Game
