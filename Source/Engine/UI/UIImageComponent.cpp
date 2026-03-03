/// @file   UIImageComponent.cpp
/// @brief  UI画像表示コンポーネント実装
#include "UIImageComponent.h"

#include "Engine/Graphics/Texture.h"
#include "Engine/Graphics/RenderSystem.h"
#include "Engine/UI/UIRectTransform.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Core/Logger.h"

namespace Engine {

    //============================================================
    // Texture
    //============================================================

    void UIImageComponent::SetTexture(std::shared_ptr<Texture> texture) {
        m_texture = std::move(texture);
    }

    std::shared_ptr<Texture> UIImageComponent::GetTexture() const {
        return m_texture;
    }

    bool UIImageComponent::HasTexture() const {
        return m_texture != nullptr && m_texture->IsLoaded();
    }

    //============================================================
    // Color
    //============================================================

    void UIImageComponent::SetColor(const Color& color) {
        m_color = color;
    }

    const Color& UIImageComponent::GetColor() const {
        return m_color;
    }

    //============================================================
    // Rendering
    //============================================================

    void UIImageComponent::Render(RenderSystem* renderSystem) {
        if (!renderSystem) {
            Logger::Warn("UIImageComponent::Render: renderSystem is null.");
            return;
        }

        if (!IsEnabled()) {
            return;
        }

        // テクスチャがロードされているか確認
        if (!m_texture) {
            Logger::Warn("UIImageComponent::Render: texture is null.");
            return;
        }

        if (!m_texture->IsLoaded()) {
            Logger::Warn("UIImageComponent::Render: texture is not loaded.");
            return;
        }

        // UIRectTransformから描画領域を取得
        GameObject* owner = GetOwner();
        if (!owner) {
            Logger::Warn("UIImageComponent::Render: owner GameObject is null.");
            return;
        }

        auto* rectTransform = owner->GetComponent<UIRectTransform>();
        if (!rectTransform) {
            Logger::Warn("UIImageComponent::Render: UIRectTransform not found on owner.");
            return;
        }

        // 矩形領域を取得
        float left, top, right, bottom;
        rectTransform->GetRect(left, top, right, bottom);

        // デバッグログ
        Logger::Trace("UIImageComponent::Render: Drawing texture at rect (" +
                      std::to_string(left) + ", " + std::to_string(top) + ") to (" +
                      std::to_string(right) + ", " + std::to_string(bottom) + ")");

        // RenderSystemのDrawSpriteを呼び出してテクスチャを描画
        renderSystem->DrawSprite(
            m_texture.get(),
            left, top, right, bottom,
            m_color.x, m_color.y, m_color.z, m_color.w
        );
    }

    //============================================================
    // Lifecycle
    //============================================================

    void UIImageComponent::OnDestroy() {
        m_texture.reset();
    }

} // namespace Engine
