/// @file   SelectUI.cpp
/// @brief  マウスクリックによるUI選択を処理するコンポーネント実装
#include "SelectUI.h"

#include "Engine/Platform/Input.h"
#include "Engine/Physics/Ray.h"
#include "Engine/Physics/Raycast.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Components/CameraComponent.h"
#include "Engine/UI/Canvas.h"
#include "Engine/UI/UIElement.h"

namespace Game {

    //============================================================
    // Lifecycle
    //============================================================

    void SelectUI::OnStart() {
        // 必要に応じて初期化処理を追加
    }

    void SelectUI::Update(float deltaTime) {
        (void)deltaTime;

        // マウス左ボタンがクリックされた瞬間を検出
        if (!Engine::Input::IsMouseTriggered(Engine::Input::MouseButton::Left)) {
            return;
        }

        // マウス座標を取得
        POINT mousePos = Engine::Input::GetMousePosition();
        float mouseX = static_cast<float>(mousePos.x);
        float mouseY = static_cast<float>(mousePos.y);

        // Canvasが設定されている場合は2D UIの当たり判定を優先
        if (m_canvas && m_canvas->IsEnabled()) {
            Engine::UIElement* hitElement = m_canvas->GetElementAt(mouseX, mouseY);
            if (hitElement && hitElement->IsEnabled()) {
                hitElement->OnClick();
                return; // UIがクリックされたので3D判定はスキップ
            }
        }

        // カメラが設定されている場合は3Dレイキャストを実行
        if (m_camera) {
            // スクリーン座標からレイを生成
            Engine::Ray ray = m_camera->ScreenPointToRay(
                mouseX, mouseY, m_screenWidth, m_screenHeight);

            // シーンに対してレイキャストを実行
            auto* owner = GetOwner();
            if (owner) {
                Engine::Scene* scene = owner->GetScene();
                if (scene) {
                    Engine::RaycastResult result;
                    if (Engine::Physics::Raycast(ray, scene, result)) {
                        // ヒットしたオブジェクトを取得
                        // NOTE: 3Dオブジェクトへのヒット処理は
                        //       派生クラスでオーバーライドするか、
                        //       別のコンポーネントで処理する
                        // Engine::GameObject* hitObject = result.GetHitObject();
                        (void)result; // 将来の拡張用
                    }
                }
            }
        }
    }

    //============================================================
    // Configuration
    //============================================================

    void SelectUI::SetCanvas(Engine::Canvas* canvas) {
        m_canvas = canvas;
    }

    void SelectUI::SetCamera(Engine::CameraComponent* camera) {
        m_camera = camera;
    }

    void SelectUI::SetScreenSize(float width, float height) {
        m_screenWidth = width;
        m_screenHeight = height;
    }

} // namespace Game
