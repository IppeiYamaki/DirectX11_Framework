#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <cstdint>
#include <string>
#include <vector>

namespace Engine {

    /// @brief  頂点入力要素の定義
    struct VertexInputElement final {
        /// @brief セマンティック名
        std::string m_semanticName;
        /// @brief セマンティックインデックス
        UINT m_semanticIndex        = 0;
        /// @brief フォーマット
        DXGI_FORMAT m_format        = DXGI_FORMAT_UNKNOWN;
        /// @brief 入力スロット
        UINT m_inputSlot            = 0;
        /// @brief バイトオフセット
        UINT m_alignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
        /// @brief 入力分類
        D3D11_INPUT_CLASSIFICATION m_inputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        /// @brief インスタンスデータステップ
        UINT m_instanceDataStepRate = 0;
    };

    /// @brief 頂点入力レイアウト
    using VertexInputLayout = std::vector<VertexInputElement>;

    /// @brief  位置/法線/色/UVのデフォルト入力レイアウトを生成
    /// @return 頂点入力レイアウト
    VertexInputLayout CreateDefaultPosNormColorUvLayout();

    /// @brief  スキニング用の入力レイアウトを生成
    /// @return 頂点入力レイアウト
    VertexInputLayout CreateSkinnedPosNormColorUvWeightsLayout();

    /// @brief  拡張頂点用（Pos/Normal/Tangent/Bitangent/UV）の入力レイアウトを生成
    /// @return 頂点入力レイアウト
    VertexInputLayout CreateExtendedPosNormTangentBitangentUvLayout();

    /// @brief  拡張スキニング用（Tangent/Bitangent含む）の入力レイアウトを生成
    /// @return 頂点入力レイアウト
    VertexInputLayout CreateExtendedSkinnedLayout();

    /// @brief 頂点シェーダーのラッパ
    class VertexShader final {
    public:
        VertexShader() = default;
        ~VertexShader();

        VertexShader(const VertexShader&) = delete;
        VertexShader& operator=(const VertexShader&) = delete;

        /// @brief  バイトコード(CSO)から頂点シェーダーを読み込む
        /// @param  device D3D11デバイス
        /// @param  csoPath CSOファイルパス
        /// @return 成功ならtrue
        bool LoadFromCSO(ID3D11Device* device, const std::wstring& csoPath);
        /// @brief 読み込み済みリソースを解放
        void Unload();

        /// @brief  読み込み済みか確認
        /// @return 読み込み済みならtrue
        bool IsLoaded() const;

        /// @brief  シェーダーを取得
        /// @return 頂点シェーダー 
        ID3D11VertexShader* GetShader() const;

        /// @brief  バイトコード先頭ポインタを取得
        /// @return バイトコード先頭ポインタ
        const void* GetBytecodeData() const;
        /// @brief  バイトコードサイズを取得
        /// @return バイトコードサイズ
        size_t GetBytecodeSize() const;

        /// @brief  読み込み元パスを取得
        /// @return ファイルパス
        const std::wstring& GetPath() const;

    private:
        std::wstring m_path{};                                  /// 読み込み元パス
        std::vector<std::uint8_t> m_bytecode;                   /// バイトコード
        Microsoft::WRL::ComPtr<ID3D11VertexShader> m_shader;    /// 頂点シェーダー
    };

    /// @brief ピクセルシェーダーのラッパ
    class PixelShader final {
    public:
        PixelShader() = default;
        ~PixelShader();

        PixelShader(const PixelShader&) = delete;
        PixelShader& operator=(const PixelShader&) = delete;

        /// @brief  バイトコード(CSO)からピクセルシェーダーを読み込む
        /// @param  device D3D11デバイス
        /// @param  csoPath CSOファイルパス
        /// @return 成功ならtrue
        bool LoadFromCSO(ID3D11Device* device, const std::wstring& csoPath);
        /// @brief 読み込み済みリソースを解放
        void Unload();

        /// @brief  読み込み済みか確認
        /// @return 読み込み済みならtrue
        bool IsLoaded() const;

        /// @brief  シェーダーを取得
        /// @return ピクセルシェーダー
        ID3D11PixelShader* GetShader() const;

        /// @brief  読み込み元パスを取得
        /// @return ファイルパス
        const std::wstring& GetPath() const;

    private:
        std::wstring m_path{};                              /// 読み込み元パス
        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_shader; /// ピクセルシェーダー
    };

    /// @brief 入力レイアウトのラッパ
    class InputLayout final {
    public:
        InputLayout() = default;
        ~InputLayout();

        InputLayout(const InputLayout&) = delete;
        InputLayout& operator=(const InputLayout&) = delete;

        /// @brief  入力レイアウトを生成
        /// @param  device D3D11デバイス
        /// @param  layout 入力レイアウト定義
        /// @param  vsBytecode 頂点シェーダーバイトコード
        /// @param  vsBytecodeSize バイトコードサイズ
        /// @return 成功ならtrue
        bool Create(
            ID3D11Device* device,
            const VertexInputLayout& layout,
            const void* vsBytecode,
            size_t vsBytecodeSize
        );

        /// @brief 入力レイアウトを解放
        void Unload();
        /// @brief  入力レイアウトが生成済みか確認
        /// @return 生成済みならtrue
        bool IsCreated() const;

        /// @brief  入力レイアウトを取得
        /// @return 入力レイアウト
        ID3D11InputLayout* GetInputLayout() const;

    private:
        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout; /// 入力レイアウト
    };

} // namespace Engine
