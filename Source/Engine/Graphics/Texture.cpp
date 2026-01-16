#include "Texture.h"

#include "Engine/Core/Logger.h"
#include "Engine/Core/Assert.h"

// DirectXTex は D3D11 ヘッダが先に必要
#include "DirectXTex.h"

#include <algorithm>

namespace Engine {

    namespace {

        bool EndsWithI(const std::wstring& s, const wchar_t* suffix) {
            const size_t sl = s.size();
            const size_t tl = wcslen(suffix);
            if (sl < tl) return false;

            for (size_t i = 0; i < tl; ++i) {
                const wchar_t a = towlower(s[sl - tl + i]);
                const wchar_t b = towlower(suffix[i]);
                if (a != b) return false;
            }
            return true;
        }

        DirectX::CREATETEX_FLAGS MakeCreateFlags(const TextureLoadOptions& opt) {
            if (opt.m_forceSRgb)  return DirectX::CREATETEX_FORCE_SRGB;
            if (opt.m_ignoreSRgb) return DirectX::CREATETEX_IGNORE_SRGB;
            return DirectX::CREATETEX_DEFAULT;
        }

    } // namespace

    Texture::~Texture() {
        Unload();
    }

    bool Texture::LoadFromFile(ID3D11Device* device, const std::wstring& filePath, const TextureLoadOptions& options) {
        if (device == nullptr) {
            Logger::Error("Texture::LoadFromFile failed: device is null.");
            return false;
        }

        // 既に読み込み済みなら一旦解放して読み直す方針
        Unload();

        m_filePath = filePath;

        DirectX::TexMetadata metadata{};
        DirectX::ScratchImage image{};

        HRESULT hr = S_OK;

        // 拡張子で分岐（DDS/TGA/WIC）
        if (EndsWithI(filePath, L".dds")) {
            hr = DirectX::LoadFromDDSFile(filePath.c_str(), DirectX::DDS_FLAGS_NONE, &metadata, image);
            if (FAILED(hr)) {
                Engine::Assert::ReportHrFailure(hr, "LoadFromDDSFile", __FILE__, __LINE__, __func__);
                Unload();
                return false;
            }
        }
        else if (EndsWithI(filePath, L".tga")) {
            hr = DirectX::LoadFromTGAFile(filePath.c_str(), DirectX::TGA_FLAGS_NONE, &metadata, image);
            if (FAILED(hr)) {
                Engine::Assert::ReportHrFailure(hr, "LoadFromTGAFile", __FILE__, __LINE__, __func__);
                Unload();
                return false;
            }
        }
        else {
            // png/jpg/bmp など（WIC）
            hr = DirectX::LoadFromWICFile(filePath.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, image);
            if (FAILED(hr)) {
                Engine::Assert::ReportHrFailure(hr, "LoadFromWICFile", __FILE__, __LINE__, __func__);
                Unload();
                return false;
            }

            // ミップ生成（WIC系のときだけ生成するのが一般的）
            if (options.m_generateMipMaps && metadata.mipLevels <= 1) {
                DirectX::ScratchImage mipChain{};
                const DirectX::Image* base = image.GetImage(0, 0, 0);
                if (base) {
                    hr = DirectX::GenerateMipMaps(*base, DirectX::TEX_FILTER_DEFAULT, 0, mipChain);
                    if (SUCCEEDED(hr)) {
                        image = std::move(mipChain);
                        metadata = image.GetMetadata();
                    }
                    else {
                        // ミップ生成失敗でも本体は使えるので Warn に留める
                        Engine::Assert::ReportHrFailure(hr, "GenerateMipMaps", __FILE__, __LINE__, __func__);
                    }
                }
            }
        }

        // SRV作成（必要なら sRGB扱いを制御）
        const auto flags = MakeCreateFlags(options);

        hr = DirectX::CreateShaderResourceViewEx(
            device,
            image.GetImages(),
            image.GetImageCount(),
            metadata,
            D3D11_USAGE_DEFAULT,
            D3D11_BIND_SHADER_RESOURCE,
            0,
            0,
            flags,
            m_srv.GetAddressOf()
        );

        if (FAILED(hr)) {
            Engine::Assert::ReportHrFailure(hr, "CreateShaderResourceViewEx", __FILE__, __LINE__, __func__);
            Unload();
            return false;
        }

        // リソースも取得しておく（必要な人向け）
        m_srv->GetResource(m_resource.GetAddressOf());

        // 情報保持
        m_width = static_cast<int>(metadata.width);
        m_height = static_cast<int>(metadata.height);
        m_mipLevels = static_cast<int>(metadata.mipLevels);
        m_format = metadata.format;

        Logger::Info("Texture loaded.");
        return true;
    }

    void Texture::Unload() {
        m_srv.Reset();
        m_resource.Reset();
        ResetInfo();
    }

    bool Texture::IsLoaded() const {
        return (m_srv != nullptr);
    }

    ID3D11ShaderResourceView* Texture::GetShaderResourceView() const {
        return m_srv.Get();
    }

    ID3D11Resource* Texture::GetResource() const {
        return m_resource.Get();
    }

    int Texture::GetWidth() const { return m_width; }
    int Texture::GetHeight() const { return m_height; }
    int Texture::GetMipLevels() const { return m_mipLevels; }
    DXGI_FORMAT Texture::GetFormat() const { return m_format; }

    const std::wstring& Texture::GetFilePath() const {
        return m_filePath;
    }

    void Texture::ResetInfo() {
        m_filePath.clear();
        m_width = 0;
        m_height = 0;
        m_mipLevels = 0;
        m_format = DXGI_FORMAT_UNKNOWN;
    }

} // namespace Engine
