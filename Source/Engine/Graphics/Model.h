#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Engine/Graphics/Mesh.h"
#include "Engine/Graphics/Material.h"

namespace Engine {

    struct ModelSubset final {
        std::uint32_t m_startIndex = 0;
        std::uint32_t m_indexCount = 0;

        std::string m_materialName{};
        std::shared_ptr<Material> m_material;
    };

    /**
     * @brief OBJ等の「1メッシュ + 複数Subset」のモデル
     */
    class Model final {
    public:
        Model() = default;
        ~Model() = default;

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        bool Initialize(
            ID3D11Device* device,
            const void* vertexData,
            std::uint32_t vertexStride,
            std::uint32_t vertexCount,
            const std::uint32_t* indexData,
            std::uint32_t indexCount,
            std::vector<ModelSubset>&& subsets
        );

        bool IsValid() const;

        Mesh& GetMesh();
        const Mesh& GetMesh() const;

        const std::vector<ModelSubset>& GetSubsets() const;

    private:
        Mesh m_mesh;
        std::vector<ModelSubset> m_subsets;
    };

} // namespace Engine
