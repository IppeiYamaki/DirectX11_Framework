# Duck PBR Shader Implementation

## Overview

This document describes the PBR (Physically Based Rendering) shader implementation for the Duck model (`Duck.obj`).

## Files

### Shader Files
- **DuckPBRVS.hlsl** - Vertex Shader
  - Transforms vertex position, normal, and UV coordinates to world space
  - Compatible with the standard `VertexPosNormColorUv` vertex format
  
- **DuckPBRPS.hlsl** - Pixel Shader
  - Implements PBR lighting with metallic/roughness workflow
  - Supports 4 texture types: Albedo, Metallic, Roughness, Normal
  - Uses derivative-based normal mapping (no tangent/bitangent required in vertex data)
  - Implements simplified PBR lighting with Fresnel, diffuse, and specular calculations

### Material Implementation
- **PlayerMaterial_Duck.cpp/h** - Material class for the Duck model
  - Loads all 4 PBR textures from the specified paths
  - Configures shader parameters and texture bindings

## Texture Setup

The shader uses the following textures:

| Slot | Type      | Path                                                      | Color Space | Description                    |
|------|-----------|-----------------------------------------------------------|-------------|--------------------------------|
| t0   | Albedo    | Textures/Characters/Duck/rubber_duck_toy_diff_4k.jpg     | sRGB        | Base color/diffuse texture     |
| t1   | Metallic  | Textures/Characters/Duck/rubber_duck_toy_metal_4k.exr    | Linear      | Metallic map (grayscale)       |
| t2   | Roughness | Textures/Characters/Duck/rubber_duck_toy_rough_4k.exr    | Linear      | Roughness map (grayscale)      |
| t3   | Normal    | Textures/Characters/Duck/rubber_duck_toy_nor_gl_4k.exr   | Linear      | Normal map (OpenGL format)     |

**Note**: Ambient Occlusion (AO) is currently a constant value set in the material parameters, not loaded from a texture file.

## Shader Features

### Material Parameters Mapping (Constant Buffer b3)
The shader uses the existing `MaterialParams` structure from `Common.hlsl` and maps PBR values as follows:

```cpp
struct MaterialParams {
    float4  BaseColor;      // Used as: Albedo base color (multiplied with texture)
    float4  Ambient;        // Used as: PBR parameters
                            //   Ambient.x = Metallic base value
                            //   Ambient.y = Roughness base value
                            //   Ambient.z = AO (Ambient Occlusion) - constant value only
                            //   Ambient.w = unused (padding)
    float4  Specular;       // Unused in PBR shader
    float4  Emissive;       // Used as: Emission color
    float   Shininess;      // Unused in PBR shader
    float   AlphaCutoff;    // Used for alpha testing
    uint    Flags;          // Material flags (which textures to use)
}
```

This mapping allows the PBR shader to work with the existing Material class without requiring a custom Material subclass.

### Material Flags
- `PBR_FLAG_USE_ALBEDO_TEX` (1 << 0) - Enable albedo texture sampling
- `PBR_FLAG_USE_NORMAL_TEX` (1 << 1) - Enable normal map
- `PBR_FLAG_USE_METALLIC_TEX` (1 << 2) - Enable metallic texture
- `PBR_FLAG_USE_ROUGHNESS_TEX` (1 << 3) - Enable roughness texture

### Lighting Model
The shader implements a simplified PBR lighting model:
- **Diffuse**: Lambert diffuse with metallic masking
- **Specular**: Blinn-Phong approximation with roughness-based shininess
- **Fresnel**: Schlick approximation for realistic reflections
- **Ambient**: Simple constant ambient lighting with AO support
- **Tone Mapping**: Reinhard tone mapping for HDR colors

### Normal Mapping
The shader uses derivative-based normal mapping, which calculates tangent and bitangent vectors in the pixel shader using `ddx()` and `ddy()` functions. This approach:
- ✅ Works with any vertex format (no tangent/bitangent required)
- ✅ Handles degenerate UVs automatically
- ⚠️ May produce artifacts at UV seams (acceptable for most models)

## Building the Shaders

The shaders are configured to compile automatically when building the Visual Studio project:
1. Open `DirectX11.sln` in Visual Studio
2. Build the project (F7)
3. The shader compiler (fxc.exe) will generate:
   - `Assets/Shaders/DuckPBRVS.cso` (compiled vertex shader)
   - `Assets/Shaders/DuckPBRPS.cso` (compiled pixel shader)

### Manual Compilation (if needed)
```cmd
fxc /T vs_5_0 /E main /Fo DuckPBRVS.cso DuckPBRVS.hlsl
fxc /T ps_5_0 /E main /Fo DuckPBRPS.cso DuckPBRPS.hlsl
```

## Usage

The `PlayerMaterial_Duck::Create()` function automatically:
1. Loads the compiled shaders (DuckPBRVS.cso, DuckPBRPS.cso)
2. Loads all 4 texture files
3. Configures the material flags
4. Returns a ready-to-use Material object

Example usage in game code:
```cpp
auto duckMaterial = PlayerMaterial_Duck::Create(materialContext);
renderer->SetMaterial(duckMaterial);
```

## Customization

### Adjusting Lighting
Edit `DuckPBRPS.hlsl` to modify lighting parameters:
```hlsl
// Line ~177: Main light direction and color
float3 mainLightDir = normalize(float3(0.5f, 1.0f, 0.3f));
float3 mainLightColor = float3(1.0f, 0.98f, 0.95f) * 1.5f;

// Line ~189: Ambient light color
float3 ambient = albedo.rgb * float3(0.3f, 0.35f, 0.4f) * g_pbrMaterial.AO;
```

### Swapping Textures
To use different textures, edit `PlayerMaterial_Duck.cpp`:
```cpp
auto albedoTex = ctx.m_assets->LoadTexture(
    L"Textures/Characters/Duck/your_custom_texture.jpg", 
    albedoOpt
);
```

### Adjusting Material Properties
Modify the base material parameters in `PlayerMaterial_Duck.cpp`:
```cpp
// Albedo base color (multiplied with texture)
mat->GetParams().m_baseColor = Engine::Vector4(1.0f, 1.0f, 1.0f, 1.0f);

// PBR parameters:
// x = Metallic (0.0 = non-metal, 1.0 = full metal)
// y = Roughness (0.0 = smooth/glossy, 1.0 = rough/matte)
// z = AO (0.0 = fully occluded, 1.0 = no occlusion)
mat->GetParams().m_ambient = Engine::Vector4(0.0f, 0.5f, 1.0f, 1.0f);
```

## Troubleshooting

### Shaders fail to load
- Ensure the .cso files are compiled and present in `Assets/Shaders/`
- Check the Visual Studio build output for shader compilation errors
- Verify shader model is set to 5.0 in the .vcxproj file

### Textures appear black
- Check that texture paths are correct
- Verify texture files exist in `Assets/Textures/Characters/Duck/`
- Ensure sRGB/Linear color space settings are correct for each texture type

### Normal mapping looks incorrect
- Verify the normal map is in OpenGL format (not DirectX format)
- Check UV coordinates are correct in the model file
- Ensure texture filtering is enabled (linear sampler)

## Technical Notes

### Material Params Reuse
The implementation cleverly reuses the existing `MaterialParams` structure instead of creating a custom PBR constant buffer:
- **Advantage**: No need for a custom Material subclass
- **Advantage**: Works with existing Material binding infrastructure
- **Trade-off**: Some fields (Specular, Shininess) are unused
- **Mapping**: Ambient.xyz stores Metallic/Roughness/AO values

### Camera Position Extraction
Since there's no dedicated camera position constant buffer, the shader extracts the camera position from the view matrix:
```hlsl
// Inverse of view matrix translation gives camera world position
float3 cameraPos = float3(
    -dot(g_view._m00_m01_m02, g_view._m30_m31_m32),
    -dot(g_view._m10_m11_m12, g_view._m30_m31_m32),
    -dot(g_view._m20_m21_m22, g_view._m30_m31_m32)
);
```

### Why Terrain Layer Textures?
The implementation uses `SetTerrainLayerTexture()` slots (t0-t3) because:
- The base `Material` class already supports multiple texture bindings
- It avoids creating a custom Material subclass
- The terrain blend buffer is not used, only the texture slots

### Color Space Handling
- **Albedo textures**: Loaded with `m_forceSRgb = true` for proper gamma correction
- **Data textures** (Metallic, Roughness, Normal): Loaded with `m_forceSRgb = false` for linear values
- **Output**: Linear color space (gamma correction handled by render target if sRGB format)

## References
- [PBR Theory](https://learnopengl.com/PBR/Theory)
- [Derivative-based Normal Mapping](http://www.thetenthplanet.de/archives/1180)
- DirectX 11 Shader Model 5.0 Specification
