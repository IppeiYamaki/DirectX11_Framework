/// @file   SceneBase.cpp
/// @brief  SceneBase共通機能の実装
#include "SceneBase.h"
#include "GameObject.h"

namespace Engine {

    void SceneBase::UpdateGameObjects(float deltaTime) {
        for (auto& obj : m_gameObjects) {
            if (obj && obj->IsActive()) {
                obj->Update(deltaTime);
            }
        }
    }

    void SceneBase::RenderGameObjects() {
        for (auto& obj : m_gameObjects) {
            if (obj && obj->IsActive()) {
                obj->Render();
            }
        }
    }

} // namespace Engine
