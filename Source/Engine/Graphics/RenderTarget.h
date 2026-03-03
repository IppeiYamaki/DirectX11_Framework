#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <cstdint>

namespace Engine {

    /// @brief オフスクリーンレンダーターゲット
    /// @note  反射パスなどのオフスクリーン描画用に使用
    class RenderTarget final {
    public:
        RenderTarget() = default;
        ~RenderTarget();

        RenderTarget(const RenderTarget&) = delete;
        RenderTarget& operator=(const RenderTarget&) = delete;

        //============================================================
        // Lifecycle
        //============================================================

        /// @brief  レンダーターゲットを生成
        /// @param  device D3D11デバイス
        /// @param  width 幅
        /// @param  height 高さ
        /// @param  format テクスチャフォーマット
        /// @return 成功ならtrue
        bool Create(ID3D11Device* device, std::uint32_t width, std::uint32_t height,
                    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);

        /// @brief リソースを解放
        void Destroy();

        /// @brief  生成済みか確認
        /// @return 生成済みならtrue
        bool IsCreated() const;

        //============================================================
        // Accessors
        //============================================================

        /// @brief  レンダーターゲットビューを取得
        /// @return RTV
        ID3D11RenderTargetView* GetRenderTargetView() const;

        /// @brief  深度ステンシルビューを取得
        /// @return DSV
        ID3D11DepthStencilView* GetDepthStencilView() const;

        /// @brief  シェーダーリソースビューを取得
        /// @return SRV
        ID3D11ShaderResourceView* GetShaderResourceView() const;

        /// @brief  テクスチャリソースを取得
        /// @return テクスチャ
        ID3D11Texture2D* GetTexture() const;

        /// @brief  幅を取得
        /// @return 幅
        std::uint32_t GetWidth() const;

        /// @brief  高さを取得
        /// @return 高さ
        std::uint32_t GetHeight() const;

        //============================================================
        // Utility
        //============================================================

        /// @brief  ビューポートを取得
        /// @return ビューポート
        D3D11_VIEWPORT GetViewport() const;

    private:
        std::uint32_t m_width = 0;
        std::uint32_t m_height = 0;
        bool m_isCreated = false;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;          ///< カラーテクスチャ
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;       ///< レンダーターゲットビュー
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;     ///< シェーダーリソースビュー

        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthTexture;     ///< 深度テクスチャ
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv;       ///< 深度ステンシルビュー

        static constexpr DXGI_FORMAT kDepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    };

} // namespace Engine
