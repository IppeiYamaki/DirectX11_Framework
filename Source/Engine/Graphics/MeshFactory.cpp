#define NOMINMAX
#include "MeshFactory.h"

#include "Engine/Graphics/MeshPrimitives.h"

namespace Engine {

    bool MeshFactory::Create(ID3D11Device* device, Mesh& outMesh, MeshType type) {
        switch (type) {
        case MeshType::Quad:    return MeshPrimitives::CreateQuad(device, outMesh, 1.0f, 1.0f);
        case MeshType::Cube:    return MeshPrimitives::CreateCube(device, outMesh, 1.0f, 1.0f, 1.0f);
        case MeshType::Plane:   return MeshPrimitives::CreatePlaneGrid(device, outMesh, 10.0f, 10.0f, 10, 10);
        case MeshType::Sphere:  return MeshPrimitives::CreateSphereUv(device, outMesh, 0.5f, 32, 16);
        case MeshType::Capsule: return MeshPrimitives::CreateCapsule(device, outMesh, 0.5f, 2.0f, 32, 8, 4);
        case MeshType::Field:   return MeshPrimitives::CreateFieldGrid(device, outMesh, 100.0f, 100.0f, 10);
        default: break;
        }
        return false;
    }

    bool MeshFactory::Create(ID3D11Device* device, Mesh& outMesh, const MeshCreateDesc& desc) {
        switch (desc.m_type) {
        case MeshType::Quad:
            return MeshPrimitives::CreateQuad(device, outMesh, desc.m_width, desc.m_height);

        case MeshType::Cube:
            return MeshPrimitives::CreateCube(device, outMesh, desc.m_width, desc.m_height, desc.m_depth);

        case MeshType::Plane:
            return MeshPrimitives::CreatePlaneGrid(device, outMesh, desc.m_width, desc.m_depth, desc.m_gridX, desc.m_gridZ);

        case MeshType::Sphere:
            if (desc.m_sphereKind == MeshCreateDesc::SphereKind::Ico)
                return MeshPrimitives::CreateSphereIco(device, outMesh, desc.m_radius, desc.m_subdivisions);
            return MeshPrimitives::CreateSphereUv(device, outMesh, desc.m_radius, desc.m_slices, desc.m_stacks);

        case MeshType::Capsule:
            return MeshPrimitives::CreateCapsule(device, outMesh, desc.m_radius, desc.m_capsuleHeight,
                desc.m_slices, desc.m_stacksHemisphere, desc.m_stacksCylinder);

        case MeshType::Field:
            return MeshPrimitives::CreateFieldGrid(device, outMesh, desc.m_width, desc.m_depth, desc.m_fieldGridSize,
                                                    desc.m_fieldAmplitude, desc.m_fieldFrequency, desc.m_fieldOctaves, desc.m_fieldSeed);

        default:
            break;
        }
        return false;
    }

} // namespace Engine
