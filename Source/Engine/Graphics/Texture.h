#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>

namespace Engine {

    /// @brief テクスチャ読み込みオプション
    struct TextureLoadOptions final {
        bool m_generateMipMaps = true;  /// ミップマップ生成フラグ
        bool m_forceSRgb = false;       /// sRGBとして強制読み込み
        bool m_ignoreSRgb = false;      /// sRGB判定を無視
    };

    /// @brief  GPUテクスチャ(SRV)のラッパ
    /// @note   LoadFromFile/Unloadでリソースを管理する
    class Texture final {
    public:
        Texture() = default;
        ~Texture();

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        //============================================================
        // Resource
        //============================================================
        /// @brief  ファイルからテクスチャを読み込む
        /// @param  device D3D11デバイス
        /// @param  filePath ファイルパス
        /// @param  options 読み込みオプション
        /// @return 成功ならtrue
        bool LoadFromFile(ID3D11Device* device, const std::wstring& filePath, const TextureLoadOptions& options = {});
        /// @brief 読み込み済みリソースを解放
        void Unload();

        /// @brief  読み込み済みか確認
        /// @return 読み込み済みならtrue
        bool IsLoaded() const;

        //============================================================
        // Getters (borrow)
        //============================================================
        /// @brief  シェーダーリソースビューを取得
        /// @return SRV
        ID3D11ShaderResourceView* GetShaderResourceView() const;
        /// @brief  リソースを取得
        /// @return リソース
        ID3D11Resource* GetResource() const;

        /// @brief  幅を取得
        /// @return 幅
        int GetWidth() const;
        /// @brief  高さを取得
        /// @return 高さ
        int GetHeight() const;
        /// @brief  ミップレベル数を取得
        /// @return ミップレベル数
        int GetMipLevels() const;
        /// @brief  フォーマットを取得
        /// @return フォーマット
        DXGI_FORMAT GetFormat() const;

        /// @brief  読み込み元パスを取得
        /// @return ファイルパス
        const std::wstring& GetFilePath() const;

    private:
        /// @brief テクスチャ情報を初期化
        void ResetInfo();

    private:
        std::wstring m_filePath{};                      /// 読み込み元パス

        int         m_width     = 0;                    /// 幅
        int         m_height    = 0;                    /// 高さ
        int         m_mipLevels = 0;                    /// ミップレベル数
        DXGI_FORMAT m_format    = DXGI_FORMAT_UNKNOWN;  /// フォーマット

        Microsoft::WRL::ComPtr<ID3D11Resource> m_resource;           /// リソース
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;      /// SRV
    };

} // namespace Engine
