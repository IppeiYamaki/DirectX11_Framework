# Perlin Noise Terrain Generation

## Overview

The `MeshPrimitives::CreateFieldGrid` function now uses Perlin Noise to generate natural-looking terrain instead of flat surfaces. This creates realistic hills, valleys, and varied landscapes.

## Features

- **Perlin Noise**: Generates smooth, natural terrain patterns
- **Fractal Noise**: Combines multiple octaves for complex terrain detail
- **Seed-based Variation**: Different seeds create different terrain layouts
- **Configurable Parameters**: Control terrain height, detail, and smoothness

## Basic Usage

### Default Terrain (Quick Start)

The simplest way to create terrain with default Perlin noise:

```cpp
// Using MeshFactory
Engine::MeshCreateDesc desc = Engine::MeshCreateDesc::Field(100.0f, 100.0f, 64);
MeshFactory::Create(device, mesh, desc);
```

This creates a 100x100 unit field with:
- Grid size: 64x64 vertices
- Amplitude: 10.0 (height variation of ±10 units)
- Frequency: 0.05 (moderate terrain detail)
- Octaves: 4 (fractal detail layers)
- Seed: 0 (default terrain pattern)

### Custom Terrain Parameters

To create custom terrain with specific characteristics:

```cpp
// Using FieldPrefab in a scene
FieldPrefab::SpawnDesc desc;
desc.m_position = Engine::Vector3(0.0f, 0.0f, 0.0f);
desc.m_width = 200.0f;              // 200 units wide
desc.m_depth = 200.0f;              // 200 units deep
desc.m_gridSize = 128;              // High detail (128x128 vertices)

// Terrain generation parameters
desc.m_terrainAmplitude = 15.0f;    // Taller hills (±15 units)
desc.m_terrainFrequency = 0.03f;    // Smoother, larger features
desc.m_terrainOctaves = 6;          // More detail layers
desc.m_terrainSeed = 12345;         // Specific terrain variation

auto* field = FieldPrefab::Spawn(ctx, desc);
```

## Parameter Guide

### m_terrainAmplitude (default: 10.0)
Controls the height variation of the terrain.
- **Low (5.0)**: Gentle rolling hills
- **Medium (10.0)**: Moderate hills and valleys
- **High (20.0)**: Dramatic mountain ranges
- **Very High (30.0+)**: Extreme, rocky terrain

### m_terrainFrequency (default: 0.05)
Controls how quickly the terrain varies (detail scale).
- **Low (0.01-0.03)**: Large, smooth features (continental scale)
- **Medium (0.05)**: Balanced terrain (default)
- **High (0.1-0.2)**: Frequent variation (hilly terrain)
- **Very High (0.3+)**: Rapid, chaotic changes

### m_terrainOctaves (default: 4, range: 1-8)
Number of noise layers combined for detail.
- **1-2**: Simple, smooth terrain
- **4**: Good balance of smoothness and detail (default)
- **6-8**: Highly detailed, complex terrain

### m_terrainSeed (default: 0)
Random seed for terrain variation.
- Same seed = same terrain pattern
- Different seed = different terrain layout
- Use for creating multiple varied levels

### m_gridSize (default: 64)
Vertex count for terrain mesh (gridSize × gridSize).
- **32**: Low detail, better performance
- **64**: Balanced (default)
- **128**: High detail, smoother terrain
- **256+**: Very high detail (performance impact)

## Example Configurations

### Rolling Hills
```cpp
desc.m_terrainAmplitude = 8.0f;
desc.m_terrainFrequency = 0.04f;
desc.m_terrainOctaves = 3;
desc.m_gridSize = 64;
```

### Mountain Terrain
```cpp
desc.m_terrainAmplitude = 25.0f;
desc.m_terrainFrequency = 0.06f;
desc.m_terrainOctaves = 6;
desc.m_gridSize = 128;
```

### Smooth Plains
```cpp
desc.m_terrainAmplitude = 3.0f;
desc.m_terrainFrequency = 0.02f;
desc.m_terrainOctaves = 2;
desc.m_gridSize = 32;
```

### Rocky, Detailed Terrain
```cpp
desc.m_terrainAmplitude = 18.0f;
desc.m_terrainFrequency = 0.1f;
desc.m_terrainOctaves = 7;
desc.m_gridSize = 128;
```

## Technical Details

### Perlin Noise Algorithm
- Based on Ken Perlin's improved noise algorithm
- Generates values in range [0, 1]
- Mapped to height range [-amplitude, +amplitude]
- Smooth, continuous gradients for natural appearance

### Fractal Noise
- Combines multiple octaves of Perlin noise
- Each octave has double the frequency and half the amplitude
- Creates multi-scale terrain detail
- Default persistence: 0.5, lacunarity: 2.0

### Normal Calculation
- Normals computed from adjacent vertices
- Ensures proper lighting on terrain
- Boundary vertices use mirrored neighbors
- Cross product of tangent vectors

## Performance Considerations

- Grid size has quadratic impact: 128x128 = 16,384 vertices
- Higher octaves increase computation time
- Balance detail vs. performance based on use case
- Typical range: 32-128 grid size for real-time applications

## Backward Compatibility

The default parameters maintain reasonable terrain generation without requiring code changes. Existing code using `CreateFieldGrid` will automatically get Perlin noise terrain with sensible defaults.

To disable terrain generation and create a flat grid:
```cpp
desc.m_terrainAmplitude = 0.0f;  // Flat terrain
```
