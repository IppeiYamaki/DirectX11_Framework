#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <cstdint>

namespace Engine {

    /// @brief  頂点/インデックスバッファを保持するメッシュ
    class Mesh final {
    public:
        Mesh() = default;
        ~Mesh();

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        /// @brief  メッシュを生成
        /// @param  device D3D11デバイス
        /// @param  vertexData 頂点データ
        /// @param  vertexStride 頂点ストライド
        /// @param  vertexCount 頂点数
        /// @param  indexData インデックスデータ
        /// @param  indexCount インデックス数
        /// @return 成功ならtrue
        bool Create(
            ID3D11Device* device,
            const void* vertexData,
            std::uint32_t vertexStride,
            std::uint32_t vertexCount,
            const std::uint32_t* indexData,
            std::uint32_t indexCount
        );

        /// @brief メッシュを破棄
        void Destroy();

        /// @brief  バッファをバインド
        /// @param  context デバイスコンテキスト
        void Bind(ID3D11DeviceContext* context) const;
        /// @brief  メッシュを描画
        /// @param  context デバイスコンテキスト
        void Draw(ID3D11DeviceContext* context) const;

        /// @brief  指定範囲を描画
        /// @param  context デバイスコンテキスト
        /// @param  indexCount 描画するインデックス数
        /// @param  startIndex 開始インデックス
        /// @param  baseVertex ベース頂点
        void DrawRange(
            ID3D11DeviceContext* context,
            std::uint32_t indexCount,
            std::uint32_t startIndex = 0,
            std::int32_t baseVertex = 0
        ) const;

        /// @brief  有効なメッシュか確認
        /// @return 有効ならtrue
        bool IsValid() const;

        /// @brief  頂点数を取得
        /// @return 頂点数
        std::uint32_t GetVertexCount() const;
        /// @brief  インデックス数を取得
        /// @return インデックス数
        std::uint32_t GetIndexCount() const;
        /// @brief  頂点ストライドを取得
        /// @return 頂点ストライド
        std::uint32_t GetVertexStride() const;

    private:
        std::uint32_t m_vertexStride = 0; ///< 頂点ストライド
        std::uint32_t m_vertexCount = 0;  ///< 頂点数
        std::uint32_t m_indexCount = 0;   ///< インデックス数

        Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer; ///< 頂点バッファ
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;  ///< インデックスバッファ
    };

} // namespace Engine
