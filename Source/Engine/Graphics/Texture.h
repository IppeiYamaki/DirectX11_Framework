#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>

namespace Engine {

    /**
     * @brief テクスチャ読み込みオプション
     */
    struct TextureLoadOptions final {
        bool m_generateMipMaps = true;  // WIC系でミップ生成する
        bool m_forceSRgb = false;       // sRGBとして扱いたい場合にtrue
        bool m_ignoreSRgb = false;      // sRGB情報を無視したい場合にtrue
    };

    /**
     * @brief GPUテクスチャ（SRV）ラッパ
     *
     * - LoadFromFile: ファイル読み込み＋SRV作成（DirectXTexを内部利用）
     * - Unload: SRV/Resource解放
     * - ComPtr統一、外へは借用raw pointer返し
     */
    class Texture final {
    public:
        Texture() = default;
        ~Texture();

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        //============================================================
        // Resource
        //============================================================
        bool LoadFromFile(ID3D11Device* device, const std::wstring& filePath, const TextureLoadOptions& options = {});
        void Unload();

        bool IsLoaded() const;

        //============================================================
        // Getters (borrow)
        //============================================================
        ID3D11ShaderResourceView* GetShaderResourceView() const;
        ID3D11Resource* GetResource() const;

        int GetWidth() const;
        int GetHeight() const;
        int GetMipLevels() const;
        DXGI_FORMAT GetFormat() const;

        const std::wstring& GetFilePath() const;

    private:
        void ResetInfo();

    private:
        std::wstring m_filePath{};

        int         m_width     = 0;                    // 幅
        int         m_height    = 0;                    // 高さ             
		int         m_mipLevels = 0;                    // ミップレベル数
        DXGI_FORMAT m_format    = DXGI_FORMAT_UNKNOWN;  // フォーマット

		Microsoft::WRL::ComPtr<ID3D11Resource>              m_resource; // 元リソース
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_srv;      // SRV
    };

} // namespace Engine
