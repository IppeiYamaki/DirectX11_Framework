/// @file   SceneBase.cpp
/// @brief  SceneBase共通機能の実装
#include "SceneBase.h"
#include "GameObject.h"

namespace Engine {

    void SceneBase::UpdateGameObjects(float deltaTime) {
        // 管理している全てのアクティブなGameObjectのUpdate()を呼び出す
        // これは軽量な更新処理であり、Scene経由の完全なライフサイクル管理とは異なる
        m_objectSlot.ForEach([deltaTime](const auto& obj) {
            if (obj && obj->IsActive()) {
                obj->Update(deltaTime);
            }
        });
    }

    void SceneBase::RenderGameObjects() {
        // 管理している全てのアクティブなGameObjectのRender()を呼び出す
        m_objectSlot.ForEach([](const auto& obj) {
            if (obj && obj->IsActive()) {
                obj->Render();
            }
        });
    }

} // namespace Engine
