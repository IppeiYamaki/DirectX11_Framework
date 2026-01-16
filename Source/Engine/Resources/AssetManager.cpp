#include "AssetManager.h"

#include "Engine/Core/Logger.h"

#include <cwctype>

namespace Engine {

    namespace {

        bool IsAbsolutePath(const std::wstring& path) {
            if (path.empty()) return false;

            // "C:\..." や "\\server\..." を雑に判定
            if (path.size() >= 2 && path[1] == L':') return true;
            if (path.size() >= 2 && path[0] == L'\\' && path[1] == L'\\') return true;
            if (path.size() >= 1 && (path[0] == L'/' || path[0] == L'\\')) return true;

            return false;
        }

        std::wstring NormalizeSlashes(std::wstring s) {
            for (auto& c : s) {
                if (c == L'\\') c = L'/';
            }
            return s;
        }

        std::wstring ToLower(std::wstring s) {
            for (auto& c : s) {
                c = static_cast<wchar_t>(std::towlower(c));
            }
            return s;
        }

    } // namespace

    AssetManager::~AssetManager() {
        Finalize();
    }

    bool AssetManager::Initialize(ID3D11Device* device) {
        if (m_isInitialized) return true;

        if (device == nullptr) {
            Logger::Error("AssetManager::Initialize failed: device is null.");
            return false;
        }

        m_device = device;

        if (!m_shaderLibrary.Initialize(m_device)) {
            Logger::Error("AssetManager::Initialize failed: ShaderLibrary initialize failed.");
            Finalize();
            return false;
        }

        m_isInitialized = true;
        return true;
    }

    void AssetManager::Finalize() {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_textureCache.clear();
        m_shaderLibrary.Finalize();

        m_device = nullptr;
        m_isInitialized = false;
    }

    void AssetManager::Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_textureCache.clear();
        m_shaderLibrary.Reset();
    }

    bool AssetManager::IsInitialized() const {
        return m_isInitialized;
    }

    void AssetManager::SetBaseDirectory(const std::wstring& baseDir) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_baseDirectory = NormalizeSlashes(baseDir);
        // 末尾に "/" が無いなら付ける（任意）
        if (!m_baseDirectory.empty() && m_baseDirectory.back() != L'/') {
            m_baseDirectory += L'/';
        }
    }

    const std::wstring& AssetManager::GetBaseDirectory() const {
        return m_baseDirectory;
    }

    std::shared_ptr<Texture> AssetManager::LoadTexture(const std::wstring& path, const TextureLoadOptions& options) {
        if (!m_isInitialized) {
            Logger::Error("AssetManager::LoadTexture failed: not initialized.");
            return nullptr;
        }

        const std::wstring resolved = ResolvePath(path);
        const std::wstring key = MakeTextureCacheKey(resolved, options);

        std::lock_guard<std::mutex> lock(m_mutex);

        // 既にキャッシュが生きているなら返す
        {
            auto it = m_textureCache.find(key);
            if (it != m_textureCache.end()) {
                if (auto shared = it->second.lock()) {
                    return shared;
                }
            }
        }

        // 新規ロード
        auto tex = std::make_shared<Texture>();
        if (!tex->LoadFromFile(m_device, resolved, options)) {
            Logger::Error("AssetManager::LoadTexture failed: Texture load failed.");
            return nullptr;
        }

        m_textureCache[key] = tex;
        return tex;
    }

    void AssetManager::PruneUnusedTextures() {
        std::lock_guard<std::mutex> lock(m_mutex);

        for (auto it = m_textureCache.begin(); it != m_textureCache.end(); ) {
            if (it->second.expired()) {
                it = m_textureCache.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    std::shared_ptr<VertexShader> AssetManager::LoadVertexShader(const std::wstring& key, const std::wstring& csoPath) {
        if (!m_isInitialized) {
            Logger::Error("AssetManager::LoadVertexShader failed: not initialized.");
            return nullptr;
        }

        const std::wstring resolved = ResolvePath(csoPath);
        return m_shaderLibrary.LoadVertexShader(key, resolved);
    }

    std::shared_ptr<PixelShader> AssetManager::LoadPixelShader(const std::wstring& key, const std::wstring& csoPath) {
        if (!m_isInitialized) {
            Logger::Error("AssetManager::LoadPixelShader failed: not initialized.");
            return nullptr;
        }

        const std::wstring resolved = ResolvePath(csoPath);
        return m_shaderLibrary.LoadPixelShader(key, resolved);
    }

    std::shared_ptr<VertexShader> AssetManager::LoadVertexShader(const std::wstring& csoPath) {
        // path=key 方式：扱いやすい（path変更で別物になる）
        const std::wstring resolved = ResolvePath(csoPath);
        return LoadVertexShader(resolved, resolved);
    }

    std::shared_ptr<PixelShader> AssetManager::LoadPixelShader(const std::wstring& csoPath) {
        const std::wstring resolved = ResolvePath(csoPath);
        return LoadPixelShader(resolved, resolved);
    }

    std::shared_ptr<InputLayout> AssetManager::CreateInputLayout(
        const std::wstring& layoutKey,
        const VertexInputLayout& layout,
        const VertexShader& vertexShader
    ) {
        if (!m_isInitialized) {
            Logger::Error("AssetManager::CreateInputLayout failed: not initialized.");
            return nullptr;
        }

        return m_shaderLibrary.CreateInputLayout(layoutKey, layout, vertexShader);
    }

    ShaderLibrary* AssetManager::GetShaderLibrary() {
        return &m_shaderLibrary;
    }

    const ShaderLibrary* AssetManager::GetShaderLibrary() const {
        return &m_shaderLibrary;
    }

    ID3D11Device* AssetManager::GetDevice() const {
        return m_device;
    }

    std::wstring AssetManager::ResolvePath(const std::wstring& path) const {
        std::wstring p = NormalizeSlashes(path);

        // 1) 絶対パスはそのまま
        if (IsAbsolutePath(p)) {
            return p;
        }

        // 2) 既に "Assets/" から始まるなら、そのまま（baseDirを二重にしない）
        //    ※ baseDir が "Assets/" の想定
        const std::wstring base = NormalizeSlashes(m_baseDirectory);
        if (!base.empty()) {
            // base は SetBaseDirectory で末尾 "/" を付けている想定
            if (p.rfind(base, 0) == 0) { // starts_with
                return p;
            }
        }

        // 3) baseDir が空ならそのまま
        if (m_baseDirectory.empty()) {
            return p;
        }

        // 4) base + relative
        return m_baseDirectory + p;
    }


    std::wstring AssetManager::MakeTextureCacheKey(const std::wstring& resolvedPath, const TextureLoadOptions& options) const {
        // オプションが違うと別テクスチャ扱いにしたいので、キーへ含める
        std::wstring key = ToLower(NormalizeSlashes(resolvedPath));

        key += L"|mip=";
        key += (options.m_generateMipMaps ? L"1" : L"0");

        key += L"|srgb=";
        key += (options.m_forceSRgb ? L"1" : L"0");

        key += L"|ignsrgb=";
        key += (options.m_ignoreSRgb ? L"1" : L"0");

        return key;
    }

} // namespace Engine
