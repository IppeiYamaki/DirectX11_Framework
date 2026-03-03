/// @file   DebugImGuiSystem.cpp
/// @brief  ImGuiベースのデバッグUI管理システム実装
#include "DebugImGuiSystem.h"

#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_impl_win32.h"
#include "ThirdParty/imgui/imgui_impl_dx11.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/LightSystem.h"
#include "Engine/Scene/Light.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Debug/DebugVisualizationSystem.h"
#include "Engine/Core/Logger.h"
#include "Engine/Math/Vector3.h"

#include "Game/GameMain.h"

// Extern declaration of message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Engine {

    DebugImGuiSystem::~DebugImGuiSystem() {
        Finalize();
    }

    bool DebugImGuiSystem::Initialize(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context) {
        if (m_isInitialized) return true;
        if (!hwnd || !device || !context) return false;

        // ImGuiコンテキストを作成
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // キーボードナビゲーション有効
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // ゲームパッドナビゲーション有効

        // ダークテーマを設定
        ImGui::StyleColorsDark();

        // Win32プラットフォームバックエンドを初期化
        if (!ImGui_ImplWin32_Init(hwnd)) {
            Logger::Error("DebugImGuiSystem::Initialize failed: ImGui_ImplWin32_Init");
            ImGui::DestroyContext();
            return false;
        }

        // DirectX11レンダリングバックエンドを初期化
        if (!ImGui_ImplDX11_Init(device, context)) {
            Logger::Error("DebugImGuiSystem::Initialize failed: ImGui_ImplDX11_Init");
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            return false;
        }

        m_isInitialized = true;
        Logger::Info("DebugImGuiSystem initialized");
        return true;
    }

    void DebugImGuiSystem::Finalize() {
        if (!m_isInitialized) return;

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        m_isInitialized = false;
        Logger::Info("DebugImGuiSystem finalized");
    }

    void DebugImGuiSystem::BeginFrame() {
        if (!m_isInitialized || !m_isEnabled) return;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void DebugImGuiSystem::Update(Scene* scene, LightSystem* lightSystem, DebugVisualizationSystem* debugVisualization) {
        if (!m_isInitialized || !m_isEnabled) return;

        // メインメニューバー
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Windows")) {
                ImGui::MenuItem("Scene Hierarchy", nullptr, &m_showSceneHierarchy);
                ImGui::MenuItem("Inspector", nullptr, &m_showInspector);
                ImGui::MenuItem("Light Manager", nullptr, &m_showLightManager);
                ImGui::MenuItem("Debug Settings", nullptr, &m_showDebugSettings);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // 各ウィンドウを描画
        if (m_showSceneHierarchy) {
            DrawSceneHierarchyWindow(scene);
        }

        if (m_showInspector) {
            DrawInspectorWindow(scene);
        }

        if (m_showLightManager) {
            DrawLightManagerWindow(lightSystem);
        }

        if (m_showDebugSettings) {
            DrawDebugSettingsWindow(debugVisualization);
        }
    }

    void DebugImGuiSystem::Render() {
        if (!m_isInitialized || !m_isEnabled) return;

        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    void DebugImGuiSystem::SetEnabled(bool enabled) {
        m_isEnabled = enabled;
    }

    bool DebugImGuiSystem::IsEnabled() const {
        return m_isEnabled;
    }

    void DebugImGuiSystem::ToggleEnabled() {
        m_isEnabled = !m_isEnabled;
    }

    bool DebugImGuiSystem::IsInitialized() const {
        return m_isInitialized;
    }

    LRESULT DebugImGuiSystem::WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        return ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam);
    }

    void DebugImGuiSystem::DrawSceneHierarchyWindow(Scene* scene) {
        ImGui::SetNextWindowSize(ImVec2(250, 400), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Scene Hierarchy", &m_showSceneHierarchy)) {
            ImGui::End();
            return;
        }

        if (!scene) {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "No Scene available");
            ImGui::End();
            return;
        }

        ImGui::Text("GameObjects: %u", scene->GetObjectCount());
        ImGui::Separator();

        // GameObject一覧を表示
        auto objects = scene->FindObjectsWhere([](GameObject*) { return true; });
        for (int i = 0; i < static_cast<int>(objects.size()); ++i) {
            auto* obj = objects[i];
            if (!obj) continue;

            const std::string& name = obj->GetName();
            std::string displayName = name.empty() ? ("GameObject_" + std::to_string(i)) : name;

            // 選択状態のハイライト
            bool isSelected = (m_selectedObjectIndex == i);

            // アクティブ状態に応じて色を変更
            if (!obj->IsActive()) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            }

            if (ImGui::Selectable(displayName.c_str(), isSelected)) {
                m_selectedObjectIndex = i;
            }

            if (!obj->IsActive()) {
                ImGui::PopStyleColor();
            }

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                Transform* transform = objects[m_selectedObjectIndex]->GetTransform();

				// カメラを選択したオブジェクトに移動
                if (transform) {
                    Vector3 targetPos = transform->GetPosition();
                    // カメラの位置をオブジェクトの位置に移動（Z軸は少し離す）
                    targetPos.z += 5.0f; // 適切な距離に調整

					// Tagが"MainCamera"のオブジェクトを探してカメラを移動
                    auto cameras = scene->FindObjectsWhere([](GameObject* obj) {
                        return obj->HasTag("MainCamera");
                    });
                    if (!cameras.empty()) {
                        Transform* cameraTransform = cameras[0]->GetTransform();
                        if (cameraTransform) {
                            cameraTransform->SetPosition(targetPos);
                        }
                    }
				}
            }
        }

        ImGui::End();
    }

    void DebugImGuiSystem::DrawInspectorWindow(Scene* scene) {
        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(270, 30), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Inspector", &m_showInspector)) {
            ImGui::End();
            return;
        }

        if (!scene || m_selectedObjectIndex < 0) {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Select a GameObject");
            ImGui::End();
            return;
        }

        auto objects = scene->FindObjectsWhere([](GameObject*) { return true; });
        if (m_selectedObjectIndex >= static_cast<int>(objects.size())) {
            m_selectedObjectIndex = -1;
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Select a GameObject");
            ImGui::End();
            return;
        }

        auto* obj = objects[m_selectedObjectIndex];
        if (!obj) {
            m_selectedObjectIndex = -1;
            ImGui::End();
            return;
        }

        // 名前
        static char nameBuffer[256] = {};
        strncpy_s(nameBuffer, obj->GetName().c_str(), sizeof(nameBuffer) - 1);
        if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
            obj->SetName(nameBuffer);
        }

        // アクティブ状態
        bool isActive = obj->IsActive();
        if (ImGui::Checkbox("Active", &isActive)) {
            obj->SetActive(isActive);
        }

        ImGui::Separator();

        // Transform
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            Transform* transform = obj->GetTransform();
            if (transform) {
                // 位置
                Vector3 pos = transform->GetPosition();
                float position[3] = { pos.x, pos.y, pos.z };
                if (ImGui::DragFloat3("Position", position, 0.1f)) {
                    transform->SetPosition(Vector3(position[0], position[1], position[2]));
                }

                // 回転（オイラー角）
                Vector3 rot = transform->GetRotationEulerDegrees();
                float rotation[3] = { rot.x, rot.y, rot.z };
                if (ImGui::DragFloat3("Rotation", rotation, 1.0f, -360.0f, 360.0f)) {
                    transform->SetRotationEulerDegrees(Vector3(rotation[0], rotation[1], rotation[2]));
                }

                // スケール
                Vector3 scl = transform->GetScale();
                float scale[3] = { scl.x, scl.y, scl.z };
                if (ImGui::DragFloat3("Scale", scale, 0.01f, 0.01f, 100.0f)) {
                    transform->SetScale(Vector3(scale[0], scale[1], scale[2]));
                }
            }
            else {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "No Transform component");
            }
        }

        // タグ
        if (ImGui::CollapsingHeader("Tags")) {
            const auto& tags = obj->GetTags();
            for (const auto& tag : tags) {
                ImGui::BulletText("%s", tag.c_str());
            }

            static char tagBuffer[128] = {};
            ImGui::InputText("##NewTag", tagBuffer, sizeof(tagBuffer));
            ImGui::SameLine();
            if (ImGui::Button("Add Tag") && tagBuffer[0] != '\0') {
                obj->AddTag(tagBuffer);
                tagBuffer[0] = '\0';
            }
        }

        // コンポーネント数
        ImGui::Separator();
        ImGui::Text("Components: %zu", obj->GetComponentCount());

        ImGui::End();
    }

    void DebugImGuiSystem::DrawLightManagerWindow(LightSystem* lightSystem) {
        ImGui::SetNextWindowSize(ImVec2(350, 450), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(580, 30), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Light Manager", &m_showLightManager)) {
            ImGui::End();
            return;
        }

        if (!lightSystem || !lightSystem->IsInitialized()) {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "LightSystem not available");
            ImGui::End();
            return;
        }

        ImGui::Text("Total Lights: %zu", lightSystem->GetLightCount());
        ImGui::Separator();

        // ライト追加ボタン
        if (ImGui::Button("Add Point Light")) {
            lightSystem->AddPointLight(Vector3(0.0f, 5.0f, 0.0f));
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Spot Light")) {
            lightSystem->AddSpotLight(Vector3(0.0f, 5.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f), 30.0f, 45.0f);
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Directional Light")) {
            lightSystem->AddDirectionalLight(Vector3(0.0f, -1.0f, 0.5f));
        }

        ImGui::Separator();

        // Directional Lights
        auto directionalLights = lightSystem->GetDirectionalLights();
        if (!directionalLights.empty() && ImGui::CollapsingHeader("Directional Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (int i = 0; i < static_cast<int>(directionalLights.size()); ++i) {
                auto* light = directionalLights[i];
                if (!light) continue;

                ImGui::PushID(reinterpret_cast<void*>(light));

                std::string label = "Directional Light " + std::to_string(i);
                if (ImGui::TreeNode(label.c_str())) {
                    // 有効/無効
                    bool enabled = light->IsEnabled();
                    if (ImGui::Checkbox("Enabled", &enabled)) {
                        if (enabled) light->Enable();
                        else light->Disable();
                    }

                    // 方向
                    Vector3 dir = light->GetDirection();
                    float direction[3] = { dir.x, dir.y, dir.z };
                    if (ImGui::DragFloat3("Direction", direction, 0.01f, -1.0f, 1.0f)) {
                        light->SetDirection(Vector3(direction[0], direction[1], direction[2]));
                    }

                    // 色
                    Vector3 col = light->GetColor();
                    float color[3] = { col.x, col.y, col.z };
                    if (ImGui::ColorEdit3("Color", color)) {
                        light->SetColor(Vector3(color[0], color[1], color[2]));
                    }

                    // 強度
                    float intensity = light->GetIntensity();
                    if (ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 10.0f)) {
                        light->SetIntensity(intensity);
                    }

                    // 環境光
                    Vector3 amb = light->GetAmbient();
                    float ambient[3] = { amb.x, amb.y, amb.z };
                    if (ImGui::ColorEdit3("Ambient", ambient)) {
                        light->SetAmbient(Vector3(ambient[0], ambient[1], ambient[2]));
                    }

                    // シャドウ
                    bool castShadow = light->IsCastShadow();
                    if (ImGui::Checkbox("Cast Shadow", &castShadow)) {
                        light->SetCastShadow(castShadow);
                    }

                    // 削除ボタン
                    if (ImGui::Button("Delete")) {
                        lightSystem->RemoveLight(light);
                    }

                    ImGui::TreePop();
                }

                ImGui::PopID();
            }
        }

        // Point Lights
        auto pointLights = lightSystem->GetPointLights();
        if (!pointLights.empty() && ImGui::CollapsingHeader("Point Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (int i = 0; i < static_cast<int>(pointLights.size()); ++i) {
                auto* light = pointLights[i];
                if (!light) continue;

                ImGui::PushID(reinterpret_cast<void*>(light));

                std::string label = "Point Light " + std::to_string(i);
                if (ImGui::TreeNode(label.c_str())) {
                    // 有効/無効
                    bool enabled = light->IsEnabled();
                    if (ImGui::Checkbox("Enabled", &enabled)) {
                        if (enabled) light->Enable();
                        else light->Disable();
                    }

                    // 位置
                    Vector3 pos = light->GetPosition();
                    float position[3] = { pos.x, pos.y, pos.z };
                    if (ImGui::DragFloat3("Position", position, 0.1f)) {
                        light->SetPosition(Vector3(position[0], position[1], position[2]));
                    }

                    // 色
                    Vector3 col = light->GetColor();
                    float color[3] = { col.x, col.y, col.z };
                    if (ImGui::ColorEdit3("Color", color)) {
                        light->SetColor(Vector3(color[0], color[1], color[2]));
                    }

                    // 強度
                    float intensity = light->GetIntensity();
                    if (ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 10.0f)) {
                        light->SetIntensity(intensity);
                    }

                    // 範囲
                    float range = light->GetRange();
                    if (ImGui::DragFloat("Range", &range, 0.1f, 0.1f, 100.0f)) {
                        light->SetRange(range);
                    }

                    // 減衰
                    float constant, linear, quadratic;
                    light->GetAttenuation(constant, linear, quadratic);
                    if (ImGui::DragFloat("Constant Attenuation", &constant, 0.01f, 0.0f, 5.0f) ||
                        ImGui::DragFloat("Linear Attenuation", &linear, 0.001f, 0.0f, 1.0f) ||
                        ImGui::DragFloat("Quadratic Attenuation", &quadratic, 0.001f, 0.0f, 1.0f)) {
                        light->SetAttenuation(constant, linear, quadratic);
                    }

                    // 削除ボタン
                    if (ImGui::Button("Delete")) {
                        lightSystem->RemoveLight(light);
                    }

                    ImGui::TreePop();
                }

                ImGui::PopID();
            }
        }

        // Spot Lights
        auto spotLights = lightSystem->GetSpotLights();
        if (!spotLights.empty() && ImGui::CollapsingHeader("Spot Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (int i = 0; i < static_cast<int>(spotLights.size()); ++i) {
                auto* light = spotLights[i];
                if (!light) continue;

                ImGui::PushID(reinterpret_cast<void*>(light));

                std::string label = "Spot Light " + std::to_string(i);
                if (ImGui::TreeNode(label.c_str())) {
                    // 有効/無効
                    bool enabled = light->IsEnabled();
                    if (ImGui::Checkbox("Enabled", &enabled)) {
                        if (enabled) light->Enable();
                        else light->Disable();
                    }

                    // 位置
                    Vector3 pos = light->GetPosition();
                    float position[3] = { pos.x, pos.y, pos.z };
                    if (ImGui::DragFloat3("Position", position, 0.1f)) {
                        light->SetPosition(Vector3(position[0], position[1], position[2]));
                    }

                    // 方向
                    Vector3 dir = light->GetDirection();
                    float direction[3] = { dir.x, dir.y, dir.z };
                    if (ImGui::DragFloat3("Direction", direction, 0.01f, -1.0f, 1.0f)) {
                        light->SetDirection(Vector3(direction[0], direction[1], direction[2]));
                    }

                    // 色
                    Vector3 col = light->GetColor();
                    float color[3] = { col.x, col.y, col.z };
                    if (ImGui::ColorEdit3("Color", color)) {
                        light->SetColor(Vector3(color[0], color[1], color[2]));
                    }

                    // 強度
                    float intensity = light->GetIntensity();
                    if (ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 10.0f)) {
                        light->SetIntensity(intensity);
                    }

                    // 範囲
                    float range = light->GetRange();
                    if (ImGui::DragFloat("Range", &range, 0.1f, 0.1f, 100.0f)) {
                        light->SetRange(range);
                    }

                    // コーン角度
                    float innerAngle, outerAngle;
                    light->GetConeAngles(innerAngle, outerAngle);
                    if (ImGui::DragFloat("Inner Angle", &innerAngle, 0.5f, 1.0f, 90.0f) ||
                        ImGui::DragFloat("Outer Angle", &outerAngle, 0.5f, 1.0f, 90.0f)) {
                        light->SetConeAngles(innerAngle, outerAngle);
                    }

                    // 減衰
                    float constant, linear, quadratic;
                    light->GetAttenuation(constant, linear, quadratic);
                    if (ImGui::DragFloat("Constant Attenuation", &constant, 0.01f, 0.0f, 5.0f) ||
                        ImGui::DragFloat("Linear Attenuation", &linear, 0.001f, 0.0f, 1.0f) ||
                        ImGui::DragFloat("Quadratic Attenuation", &quadratic, 0.001f, 0.0f, 1.0f)) {
                        light->SetAttenuation(constant, linear, quadratic);
                    }

                    // 削除ボタン
                    if (ImGui::Button("Delete")) {
                        lightSystem->RemoveLight(light);
                    }

                    ImGui::TreePop();
                }

                ImGui::PopID();
            }
        }

        ImGui::End();
    }

    void DebugImGuiSystem::DrawDebugSettingsWindow(DebugVisualizationSystem* debugVisualization) {
        ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(940, 30), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Debug Settings", &m_showDebugSettings)) {
            ImGui::End();
            return;
        }

        if (!debugVisualization || !debugVisualization->IsInitialized()) {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "DebugVisualization not available");
            ImGui::End();
            return;
        }

        // 可視化タイプ
        ImGui::Text("Visualization Settings");
        ImGui::Separator();

        bool visualizationEnabled = debugVisualization->IsEnabled();
        if (ImGui::Checkbox("Visualization Enabled", &visualizationEnabled)) {
            debugVisualization->SetEnabled(visualizationEnabled);
        }

        // 可視化タイプ選択（ビットフラグ形式に対応）
        static const char* visualizationTypes[] = { "None", "Outline Only", "Light Spheres Only", "All (Outline + Spheres)" };
        int currentType = static_cast<int>(debugVisualization->GetVisualizationType());
        // 現在のタイプをドロップダウンインデックスに変換
        int comboIndex = 0;
        if (currentType == 0) comboIndex = 0;          // None
        else if (currentType == 1) comboIndex = 1;     // Outline
        else if (currentType == 4) comboIndex = 2;     // LightSphere
        else comboIndex = 3;                           // All (5) or other combinations
        
        if (ImGui::Combo("Type", &comboIndex, visualizationTypes, IM_ARRAYSIZE(visualizationTypes))) {
            // ドロップダウンインデックスをDebugVisualizationTypeに変換
            DebugVisualizationType newType = DebugVisualizationType::None;
            switch (comboIndex) {
                case 0: newType = DebugVisualizationType::None; break;
                case 1: newType = DebugVisualizationType::Outline; break;
                case 2: newType = DebugVisualizationType::LightSphere; break;
                case 3: newType = DebugVisualizationType::All; break;
            }
            debugVisualization->SetVisualizationType(newType);
        }

        ImGui::Separator();

        // アウトライン設定
        if (ImGui::CollapsingHeader("Outline Settings")) {
            DirectX::XMFLOAT3 outlineColor = debugVisualization->GetOutlineColor();
            float color[3] = { outlineColor.x, outlineColor.y, outlineColor.z };
            if (ImGui::ColorEdit3("Outline Color", color)) {
                debugVisualization->SetOutlineColor(DirectX::XMFLOAT3(color[0], color[1], color[2]));
            }

            float thickness = debugVisualization->GetOutlineThickness();
            if (ImGui::DragFloat("Outline Thickness", &thickness, 0.001f, 0.001f, 0.1f)) {
                debugVisualization->SetOutlineThickness(thickness);
            }
        }

        // ライト球体設定（Light Sphere Settings）
        if (ImGui::CollapsingHeader("Light Sphere Settings")) {
            float sphereSize = debugVisualization->GetLightSphereSize();
            if (ImGui::DragFloat("Sphere Size", &sphereSize, 0.05f, 0.1f, 5.0f)) {
                debugVisualization->SetLightSphereSize(sphereSize);
            }

            DirectX::XMFLOAT3 dirLightPos = debugVisualization->GetDirectionalLightSpherePosition();
            float position[3] = { dirLightPos.x, dirLightPos.y, dirLightPos.z };
            if (ImGui::DragFloat3("Dir Light Sphere Position", position, 0.5f)) {
                debugVisualization->SetDirectionalLightSpherePosition(DirectX::XMFLOAT3(position[0], position[1], position[2]));
            }
        }

        ImGui::End();
    }

} // namespace Engine
