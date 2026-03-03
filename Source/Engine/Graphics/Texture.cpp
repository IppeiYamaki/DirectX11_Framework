#include "Texture.h"

#include "Engine/Core/Logger.h"
#include "Engine/Core/Assert.h"

// DirectXTex �� D3D11 �w�b�_����ɕK�v
#include "ThirdParty/DirectXTex/DirectXTex.h"

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

        // ���ɓǂݍ��ݍς݂Ȃ��U������ēǂݒ������j
        Unload();

        m_filePath = filePath;

        DirectX::TexMetadata metadata{};
        DirectX::ScratchImage image{};

        HRESULT hr = S_OK;

        // �g���q�ŕ���iDDS/TGA/WIC�j
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
            // png/jpg/bmp �ȂǁiWIC�j
            hr = DirectX::LoadFromWICFile(filePath.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, image);
            if (FAILED(hr)) {
                Engine::Assert::ReportHrFailure(hr, "LoadFromWICFile", __FILE__, __LINE__, __func__);
                Unload();
                return false;
            }

            // �~�b�v�����iWIC�n�̂Ƃ�������������̂���ʓI�j
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
                        // �~�b�v�������s�ł��{�͎̂g����̂� Warn �ɗ��߂�
                        Engine::Assert::ReportHrFailure(hr, "GenerateMipMaps", __FILE__, __LINE__, __func__);
                    }
                }
            }
        }

        // SRV�쐬�i�K�v�Ȃ� sRGB�����𐧌�j
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

        // ���\�[�X���擾���Ă����i�K�v�Ȑl�����j
        m_srv->GetResource(m_resource.GetAddressOf());

        // ���ێ�
        m_width = static_cast<int>(metadata.width);
        m_height = static_cast<int>(metadata.height);
        m_mipLevels = static_cast<int>(metadata.mipLevels);
        m_format = metadata.format;

        // Convert wide string to narrow for logging
        std::string narrowPath(filePath.begin(), filePath.end());
        Logger::Info("Texture loaded: " + narrowPath + " (" + 
            std::to_string(m_width) + "x" + std::to_string(m_height) + 
            "), SRV=" + std::to_string(reinterpret_cast<uintptr_t>(m_srv.Get())));
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
