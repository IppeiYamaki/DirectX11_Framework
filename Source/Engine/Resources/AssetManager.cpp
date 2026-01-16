#include "AssetManager.h"

#include "Engine/Core/Logger.h"
#include "Engine/Resources/ObjModelLoader.h"

#include <cwctype>
#include <filesystem>

namespace Engine {

    namespace {

        bool IsAbsolutePath(const std::wstring& path) {
            if (path.empty()) return false;

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
        m_modelCache.clear();

        m_shaderLibrary.Finalize();

        m_device = nullptr;
        m_isInitialized = false;
    }

    void AssetManager::Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_textureCache.clear();
        m_modelCache.clear();

        m_shaderLibrary.Reset();
    }

    bool AssetManager::IsInitialized() const {
        return m_isInitialized;
    }

    void AssetManager::SetBaseDirectory(const std::wstring& baseDir) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_baseDirectory = NormalizeSlashes(baseDir);
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

        {
            auto it = m_textureCache.find(key);
            if (it != m_textureCache.end()) {
                if (auto shared = it->second.lock()) {
                    return shared;
                }
            }
        }

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

    std::shared_ptr<Model> AssetManager::LoadModel(const std::wstring& path) {
        if (!m_isInitialized) {
            Logger::Error("AssetManager::LoadModel failed: not initialized.");
            return nullptr;
        }

        const std::wstring resolved = ResolvePath(path);
        const std::wstring key = MakeModelCacheKey(resolved);

        // 1) cache check（ここはロック）
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_modelCache.find(key);
            if (it != m_modelCache.end()) {
                if (auto shared = it->second.lock()) {
                    return shared;
                }
            }
        }

        // 2) load（ロック外：LoadTextureが内部ロックするのでデッドロック回避）
        const std::filesystem::path p(resolved);
        const std::wstring ext = ToLower(p.extension().wstring());

        std::shared_ptr<Model> model;
        if (ext == L".obj") {
            model = LoadObjModel(m_device, *this, resolved);
        }
        else {
            Logger::Error("AssetManager::LoadModel failed: unsupported extension (only .obj).");
            return nullptr;
        }

        if (!model) {
            Logger::Error("AssetManager::LoadModel failed: loader returned null.");
            return nullptr;
        }

        // 3) store cache
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_modelCache[key] = model;
        }

        return model;
    }

    void AssetManager::PruneUnusedModels() {
        std::lock_guard<std::mutex> lock(m_mutex);

        for (auto it = m_modelCache.begin(); it != m_modelCache.end(); ) {
            if (it->second.expired()) {
                it = m_modelCache.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    ID3D11Device* AssetManager::GetDevice() const {
        return m_device;
    }

    std::wstring AssetManager::ResolvePath(const std::wstring& path) const {
        std::wstring p = NormalizeSlashes(path);

        if (IsAbsolutePath(p)) {
            return p;
        }

        const std::wstring base = NormalizeSlashes(m_baseDirectory);
        if (!base.empty()) {
            if (p.rfind(base, 0) == 0) {
                return p;
            }
        }

        if (m_baseDirectory.empty()) {
            return p;
        }

        return m_baseDirectory + p;
    }

    std::wstring AssetManager::MakeTextureCacheKey(const std::wstring& resolvedPath, const TextureLoadOptions& options) const {
        std::wstring key = ToLower(NormalizeSlashes(resolvedPath));

        key += L"|mip=";
        key += (options.m_generateMipMaps ? L"1" : L"0");

        key += L"|srgb=";
        key += (options.m_forceSRgb ? L"1" : L"0");

        key += L"|ignsrgb=";
        key += (options.m_ignoreSRgb ? L"1" : L"0");

        return key;
    }

    std::wstring AssetManager::MakeModelCacheKey(const std::wstring& resolvedPath) const {
        // 今はオプション無しなのでパスだけ
        return ToLower(NormalizeSlashes(resolvedPath));
    }

} // namespace Engine
