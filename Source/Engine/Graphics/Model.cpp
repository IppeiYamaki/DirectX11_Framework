#include "Model.h"

#include "Engine/Core/Logger.h"

namespace Engine {

    bool Model::Initialize(
        ID3D11Device* device,
        const void* vertexData,
        std::uint32_t vertexStride,
        std::uint32_t vertexCount,
        const std::uint32_t* indexData,
        std::uint32_t indexCount,
        std::vector<ModelSubset>&& subsets
    ) {
        if (!device) {
            Logger::Error("Model::Initialize failed: device is null.");
            return false;
        }
        if (!vertexData || vertexStride == 0 || vertexCount == 0) {
            Logger::Error("Model::Initialize failed: vertex is invalid.");
            return false;
        }
        if (!indexData || indexCount == 0) {
            Logger::Error("Model::Initialize failed: index is invalid.");
            return false;
        }

        m_mesh.Destroy();
        if (!m_mesh.Create(device, vertexData, vertexStride, vertexCount, indexData, indexCount)) {
            Logger::Error("Model::Initialize failed: Mesh::Create failed.");
            return false;
        }

        m_subsets = std::move(subsets);

        // Subsetが空なら全体を1つにする（安全策）
        if (m_subsets.empty()) {
            ModelSubset s{};
            s.m_startIndex = 0;
            s.m_indexCount = indexCount;
            s.m_materialName = "Default";
            s.m_material = nullptr;
            m_subsets.push_back(std::move(s));
        }

        return true;
    }

    bool Model::IsValid() const {
        return m_mesh.IsValid() && (m_mesh.GetIndexCount() > 0);
    }

    Mesh& Model::GetMesh() { return m_mesh; }
    const Mesh& Model::GetMesh() const { return m_mesh; }

    const std::vector<ModelSubset>& Model::GetSubsets() const { return m_subsets; }

} // namespace Engine
