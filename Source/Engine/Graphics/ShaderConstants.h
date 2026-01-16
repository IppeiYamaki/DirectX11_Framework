#pragma once

#include <cstdint>
#include <DirectXMath.h>

#include "Engine/Math/Vector4.h"

namespace Engine {

    static constexpr std::uint32_t kMaterialFlagUseTexture = 1u << 0;
    static constexpr std::uint32_t kMaterialFlagAlphaTest = 1u << 1;

    static constexpr std::uint32_t kLightFlagEnabled = 1u << 0;

    // スキニング
    static constexpr std::uint32_t kMaxBones = 128;

    struct alignas(16) WorldCB final { DirectX::XMFLOAT4X4 g_world{}; };
    struct alignas(16) ViewCB final { DirectX::XMFLOAT4X4 g_view{}; };
    struct alignas(16) ProjectionCB final { DirectX::XMFLOAT4X4 g_projection{}; };

    struct alignas(16) MaterialParams final {
        Vector4 m_baseColor{ 1,1,1,1 };
        Vector4 m_ambient{ 1,1,1,1 };
        Vector4 m_specular{ 0,0,0,0 };
        Vector4 m_emissive{ 0,0,0,0 };

        float m_shininess = 0.0f;
        float m_alphaCutoff = 0.5f;
        std::uint32_t m_flags = 0;
        std::uint32_t m_pad0 = 0;
    };

    struct alignas(16) MaterialCB final { MaterialParams g_material{}; };

    struct alignas(16) DirectionalLight final {
        float m_directionX = 0.0f;
        float m_directionY = -1.0f;
        float m_directionZ = 0.0f;
        std::uint32_t m_flags = 0;

        Vector4 m_diffuse{ 1,1,1,1 };
        Vector4 m_ambient{ 0.2f,0.2f,0.2f,1.0f };
    };

    struct alignas(16) LightCB final { DirectionalLight g_light{}; };

    // ★b5 : Bones
    struct alignas(16) BonesCB final {
        DirectX::XMFLOAT4X4 g_bones[kMaxBones]{};
    };

    inline DirectX::XMFLOAT4X4 MakeIdentityMatrix() {
        DirectX::XMFLOAT4X4 m{};
        DirectX::XMStoreFloat4x4(&m, DirectX::XMMatrixIdentity());
        return m;
    }

} // namespace Engine
