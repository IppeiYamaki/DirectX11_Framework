/// @file   Canvas.cpp
/// @brief  UI要素を管理するCanvasクラス実装
#include "Canvas.h"

#include "Engine/UI/UIElement.h"
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
        m_isEnabled = true;
        m_needsSort = false;

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

        // 必要に応じてソート
        if (m_needsSort) {
            SortElements();
            m_needsSort = false;
        }

        // UI要素を描画（順序通り）
        for (auto& element : m_elements) {
            if (element && element->IsEnabled() && element->IsVisible()) {
                element->Render(renderSystem);
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
    // Private
    //============================================================

    void Canvas::SortElements() {
        std::stable_sort(m_elements.begin(), m_elements.end(),
            [](const std::unique_ptr<UIElement>& a, const std::unique_ptr<UIElement>& b) {
                return a->GetSortOrder() < b->GetSortOrder();
            });
    }

} // namespace Engine
