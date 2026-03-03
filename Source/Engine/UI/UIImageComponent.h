/// @file   UIImageComponent.h
/// @brief  UI画像表示コンポーネント（Unity風）
#pragma once

#include "Engine/Scene/Component.h"
#include "Engine/Math/Color.h"

#include <memory>

namespace Engine {

    class Texture;
    class RenderSystem;

    /// @brief UI画像表示コンポーネント
    /// @note  UIRectTransformの領域にテクスチャを描画する
    class UIImageComponent : public Component {
    public:
        UIImageComponent() = default;
        ~UIImageComponent() override = default;

        //============================================================
        // Texture
        //============================================================

        /// @brief テクスチャを設定
        /// @param texture 設定するテクスチャ（nullptrで解除）
        void SetTexture(std::shared_ptr<Texture> texture);

        /// @brief テクスチャを取得
        /// @return テクスチャ
        [[nodiscard]] std::shared_ptr<Texture> GetTexture() const;

        /// @brief テクスチャが設定されているか確認
        /// @return テクスチャが設定されていればtrue
        [[nodiscard]] bool HasTexture() const;

        //============================================================
        // Color
        //============================================================

        /// @brief 色（乗算カラー）を設定
        /// @param color 色
        void SetColor(const Color& color);

        /// @brief 色を取得
        /// @return 色
        [[nodiscard]] const Color& GetColor() const;

        //============================================================
        // Rendering
        //============================================================

        /// @brief UI画像を描画
        /// @param renderSystem RenderSystem参照
        /// @note  このメソッドはCanvas側から呼び出される
        void Render(RenderSystem* renderSystem);

        //============================================================
        // Lifecycle (Component overrides)
        //============================================================

        void OnDestroy() override;

    private:
        std::shared_ptr<Texture> m_texture;                 ///< 表示するテクスチャ
        Color m_color{ 1.0f, 1.0f, 1.0f, 1.0f };           ///< 乗算カラー
    };

} // namespace Engine
