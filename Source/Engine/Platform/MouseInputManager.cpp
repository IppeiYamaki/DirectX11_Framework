/// @file   MouseInputManager.cpp
/// @brief  マウス入力イベントを管理するクラス実装
#include "MouseInputManager.h"

#include "Engine/Platform/Input.h"

namespace Engine {

    //============================================================
    // Listener Registration
    //============================================================

    void MouseInputManager::RegisterClickListener(const ClickListener& listener) {
        if (listener) {
            m_clickListeners.push_back(listener);
        }
    }

    void MouseInputManager::ClearClickListeners() {
        m_clickListeners.clear();
    }

    std::size_t MouseInputManager::GetListenerCount() const {
        return m_clickListeners.size();
    }

    //============================================================
    // Frame Update
    //============================================================

    void MouseInputManager::Update() {
        // マウス左ボタンがクリックされた瞬間を検出
        if (Input::IsMouseTriggered(Input::MouseButton::Left)) {
            POINT mousePos = Input::GetMousePosition();
            float x = static_cast<float>(mousePos.x);
            float y = static_cast<float>(mousePos.y);

            // 登録されている全リスナーに通知
            for (const auto& listener : m_clickListeners) {
                if (listener) {
                    listener(x, y);
                }
            }
        }
    }

} // namespace Engine
