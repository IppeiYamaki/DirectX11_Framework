/// @file   Canvas.cpp
/// @brief  UI要素を管理するCanvasクラス実装
#include "Canvas.h"

#include "Engine/UI/UIElement.h"
#include "Engine/UI/UIRectTransform.h"
#include "Engine/UI/UIImageComponent.h"
#include "Engine/UI/UIButtonComponent.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Core/Logger.h"

#include <algorithm>

namespace Engine {

    Canvas::~Canvas() {
        Finalize();
    }

    //============================================================
    // Lifecycle
    //============================================================

    bool Canvas::Initialize(float screenWidth, float screenHeight) {
        if (m_isInitialized) return true;

        m_screenWidth = screenWidth;
        m_screenHeight = screenHeight;
        m_elements.clear();
        m_uiObjects.clear();
        m_isEnabled = true;
        m_needsSort = false;
        m_needsUIObjectSort = false;

        m_isInitialized = true;
        Logger::Info("Canvas initialized.");
        return true;
    }

    void Canvas::Finalize() {
        // 初期化状態に関わらず、要素があれば破棄処理を行う
        for (auto& element : m_elements) {
            if (element) {
                element->OnDestroy();
            }
        }
        m_elements.clear();

        // UI GameObjectは借用なので、参照をクリアするだけ
        m_uiObjects.clear();

        if (!m_isInitialized) {
            return;
        }

        m_isInitialized = false;
        Logger::Info("Canvas finalized.");
    }

    //============================================================
    // Element Management
    //============================================================

    UIElement* Canvas::AddElement(std::unique_ptr<UIElement> element) {
        if (!m_isInitialized || !element) {
            return nullptr;
        }

        UIElement* raw = element.get();
        raw->SetCanvas(this);
        raw->Initialize();

        m_elements.emplace_back(std::move(element));
        m_needsSort = true;

        return raw;
    }

    void Canvas::RemoveElement(UIElement* element) {
        if (!m_isInitialized || !element) return;

        for (auto it = m_elements.begin(); it != m_elements.end(); ++it) {
            if (it->get() == element) {
                (*it)->OnDestroy();
                m_elements.erase(it);
                return;
            }
        }
    }

    void Canvas::ClearElements() {
        if (!m_isInitialized) return;

        for (auto& element : m_elements) {
            if (element) {
                element->OnDestroy();
            }
        }
        m_elements.clear();
    }

    std::size_t Canvas::GetElementCount() const {
        return m_elements.size();
    }

    //============================================================
    // Frame
    //============================================================

    void Canvas::Update(float deltaTime) {
        if (!m_isInitialized || !m_isEnabled) return;

        for (auto& element : m_elements) {
            if (element && element->IsEnabled()) {
                element->Update(deltaTime);
            }
        }
    }

    void Canvas::Render(RenderSystem* renderSystem) {
        if (!m_isInitialized || !m_isEnabled || !renderSystem) return;

        // デバッグログ: Canvas::Render が呼ばれた
        Logger::Trace("Canvas::Render called. UIObject count: " + std::to_string(m_uiObjects.size()));

        // 必要に応じてソート（Legacy UI要素）
        if (m_needsSort) {
            SortElements();
            m_needsSort = false;
        }

        // 必要に応じてソート（UI GameObjects）
        if (m_needsUIObjectSort) {
            SortUIObjects();
            m_needsUIObjectSort = false;
        }

        // Legacy UI要素を描画（順序通り）
        for (auto& element : m_elements) {
            if (element && element->IsEnabled() && element->IsVisible()) {
                element->Render(renderSystem);
            }
        }

        // UI GameObjectsの描画（UIImageComponentを通じて）
        for (auto* uiObj : m_uiObjects) {
            if (uiObj && uiObj->IsActive()) {
                auto* imageComp = uiObj->GetComponent<UIImageComponent>();
                if (imageComp && imageComp->IsEnabled()) {
                    Logger::Trace("Canvas::Render: Rendering UIImageComponent for '" + uiObj->GetName() + "'");
                    imageComp->Render(renderSystem);
                }
            }
        }
    }

    //============================================================
    // Screen
    //============================================================

    void Canvas::SetScreenSize(float width, float height) {
        m_screenWidth = width;
        m_screenHeight = height;
    }

    float Canvas::GetScreenWidth() const {
        return m_screenWidth;
    }

    float Canvas::GetScreenHeight() const {
        return m_screenHeight;
    }

    //============================================================
    // State
    //============================================================

    bool Canvas::IsInitialized() const {
        return m_isInitialized;
    }

    void Canvas::Enable() {
        m_isEnabled = true;
    }

    void Canvas::Disable() {
        m_isEnabled = false;
    }

    bool Canvas::IsEnabled() const {
        return m_isEnabled;
    }

    //============================================================
    // Mouse Input
    //============================================================

    bool Canvas::HandleMouseClick(float mouseX, float mouseY) {
        if (!m_isInitialized || !m_isEnabled) {
            return false;
        }

        // ソート順序で後ろ（手前に表示されている）から順にチェック
        if (m_needsSort) {
            SortElements();
            m_needsSort = false;
        }

        // 逆順にイテレート（手前の要素から順にチェック）
        for (auto it = m_elements.rbegin(); it != m_elements.rend(); ++it) {
            auto& element = *it;
            if (element && element->IsEnabled() && element->IsVisible()) {
                if (element->Contains(mouseX, mouseY)) {
                    element->OnClick();
                    return true;
                }
            }
        }

        return false;
    }

    UIElement* Canvas::GetElementAt(float mouseX, float mouseY) const {
        if (!m_isInitialized || !m_isEnabled) {
            return nullptr;
        }

        // 逆順にイテレート（手前の要素から順にチェック）
        for (auto it = m_elements.rbegin(); it != m_elements.rend(); ++it) {
            const auto& element = *it;
            if (element && element->IsEnabled() && element->IsVisible()) {
                if (element->Contains(mouseX, mouseY)) {
                    return element.get();
                }
            }
        }

        return nullptr;
    }

    //============================================================
    // Private
    //============================================================

    void Canvas::SortElements() {
        std::stable_sort(m_elements.begin(), m_elements.end(),
            [](const std::unique_ptr<UIElement>& a, const std::unique_ptr<UIElement>& b) {
                return a->GetSortOrder() < b->GetSortOrder();
            });
    }

    //============================================================
    // UI GameObject Management
    //============================================================

    void Canvas::AddUIObject(GameObject* uiObject) {
        if (!m_isInitialized || !uiObject) {
            return;
        }

        // 既に登録されているかチェック
        auto it = std::find(m_uiObjects.begin(), m_uiObjects.end(), uiObject);
        if (it != m_uiObjects.end()) {
            return;
        }

        // UIRectTransformコンポーネントが必須
        if (!uiObject->GetComponent<UIRectTransform>()) {
            Logger::Warn("Canvas::AddUIObject: GameObject does not have UIRectTransform component.");
            return;
        }

        m_uiObjects.push_back(uiObject);
        m_needsUIObjectSort = true;

        Logger::Info("Canvas: UI GameObject '" + uiObject->GetName() + "' added.");
    }

    void Canvas::RemoveUIObject(GameObject* uiObject) {
        if (!m_isInitialized || !uiObject) {
            return;
        }

        auto it = std::find(m_uiObjects.begin(), m_uiObjects.end(), uiObject);
        if (it != m_uiObjects.end()) {
            m_uiObjects.erase(it);
            Logger::Info("Canvas: UI GameObject '" + uiObject->GetName() + "' removed.");
        }
    }

    void Canvas::ClearUIObjects() {
        m_uiObjects.clear();
    }

    std::size_t Canvas::GetUIObjectCount() const {
        return m_uiObjects.size();
    }

    GameObject* Canvas::GetUIObjectAt(float mouseX, float mouseY) const {
        if (!m_isInitialized || !m_isEnabled) {
            return nullptr;
        }

        // 逆順にイテレート（手前の要素から順にチェック）
        for (auto it = m_uiObjects.rbegin(); it != m_uiObjects.rend(); ++it) {
            GameObject* uiObj = *it;
            if (uiObj && uiObj->IsActive()) {
                auto* rectTransform = uiObj->GetComponent<UIRectTransform>();
                if (rectTransform && rectTransform->IsEnabled()) {
                    if (rectTransform->Contains(mouseX, mouseY)) {
                        return uiObj;
                    }
                }
            }
        }

        return nullptr;
    }

    void Canvas::HandleUIObjectMouseInput(float mouseX, float mouseY, bool isPressed) {
        if (!m_isInitialized || !m_isEnabled) {
            return;
        }

        // 必要に応じてソート
        if (m_needsUIObjectSort) {
            SortUIObjects();
            m_needsUIObjectSort = false;
        }

        // 全UI GameObjectのボタンコンポーネントにマウス入力を渡す
        for (auto* uiObj : m_uiObjects) {
            if (uiObj && uiObj->IsActive()) {
                auto* rectTransform = uiObj->GetComponent<UIRectTransform>();
                auto* buttonComp = uiObj->GetComponent<UIButtonComponent>();

                if (rectTransform && buttonComp && 
                    rectTransform->IsEnabled() && buttonComp->IsEnabled()) {
                    bool isInside = rectTransform->Contains(mouseX, mouseY);
                    buttonComp->HandleMouseInput(isInside, isPressed);
                }
            }
        }
    }

    void Canvas::SortUIObjects() {
        std::stable_sort(m_uiObjects.begin(), m_uiObjects.end(),
            [](GameObject* a, GameObject* b) {
                auto* rectA = a ? a->GetComponent<UIRectTransform>() : nullptr;
                auto* rectB = b ? b->GetComponent<UIRectTransform>() : nullptr;
                int orderA = rectA ? rectA->GetSortOrder() : 0;
                int orderB = rectB ? rectB->GetSortOrder() : 0;
                return orderA < orderB;
            });
    }

} // namespace Engine
