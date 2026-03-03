#include "SkyFollowCameraComponent.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/CameraComponent.h"
#include "Engine/Scene/Components/Transform.h"

namespace Game {

    void SkyFollowCameraComponent::Update(float deltaTime) {
        // MainCamera取得 (SceneからCameraComponentを探す)
        auto* scene = GetGameObject()->GetScene();

        // Tagが"MainCamera"のオブジェクトを探してカメラを移動
        auto mainCameraObj = scene->FindObjectsWhere([](Engine::GameObject* obj) {
            return obj->GetTag() == "MainCamera";
			});


        if (!mainCameraObj.empty()) {
            auto* cameraTransform = mainCameraObj[0]->GetComponent<Engine::Transform>();
            auto* skyTransform = GetGameObject()->GetComponent<Engine::Transform>();
            if (cameraTransform && skyTransform) {
                // カメラ位置をSkyにコピー（rotation/scaleはコピーしなくてOK）
                skyTransform->SetPosition(cameraTransform->GetPosition());
            }
        }
    }

} // namespace Game
