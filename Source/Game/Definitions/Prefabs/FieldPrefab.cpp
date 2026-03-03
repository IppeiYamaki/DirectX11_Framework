/// @file   FieldPrefab.cpp
/// @brief  地形（フィールド）用Prefabクラスの実装
#include "FieldPrefab.h"

#include "Engine/Scene/SceneContext.h"

#include <algorithm>
#include <typeinfo>

#include "Engine/Core/Logger.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Components/MeshRenderer.h"
#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Graphics/MeshFactory.h"
#include "Engine/Graphics/Material.h"
#include "Engine/Resources/AssetManager.h"
#include "Engine/Materials/MaterialLibrary.h"
#include "Engine/Materials/MaterialBuildContext.h"
#include "Engine/Materials/TerrainBlendMaterial.h"
#include "Engine/Math/MathConstants.h"

#include "Game/GameObjects/Field.h"
#include "Game/Definitions/Materials/SampleCubeMaterial.h"

namespace Game {

    Engine::GameObject* FieldPrefab::Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_scene || !ctx.m_renderSystem || !ctx.m_device) {
            Engine::Logger::Error("FieldPrefab::Spawn failed: ctx invalid.");
            return nullptr;
        }

        // レガシーFieldを使用する場合
        if (desc.m_useLegacyField || !desc.m_useMeshRenderer) {
            return SpawnLegacyField(ctx, desc);
        }

        // MeshRendererを使用する新しい方式
        return SpawnWithMeshRenderer(ctx, desc);
    }

    Engine::GameObject* FieldPrefab::SpawnWithMeshRenderer(Engine::SceneContext& ctx, const SpawnDesc& desc) {
        // GameObjectを生成
        auto* obj = ctx.m_scene->CreateObject<Engine::GameObject>("Field");
        if (!obj) {
            Engine::Logger::Error("FieldPrefab::SpawnWithMeshRenderer failed: CreateObject returned nullptr.");
            return nullptr;
        }

        // Transform設定
        if (auto* tr = obj->GetComponent<Engine::Transform>()) {
            tr->SetPosition(desc.m_position);
            tr->SetRotationEulerDegrees(Engine::Vector3(
                Engine::Math::DegreesToRadians(desc.m_rotation.x),
                Engine::Math::DegreesToRadians(desc.m_rotation.y),
                Engine::Math::DegreesToRadians(desc.m_rotation.z)
            ));
            tr->SetUniformScale(desc.m_uniformScale);
        }

        // MeshRenderer追加
        auto* mr = obj->AddComponent<Engine::MeshRenderer>(ctx.m_device, ctx.m_renderSystem);
        if (!mr) {
            Engine::Logger::Error("FieldPrefab::SpawnWithMeshRenderer failed: AddComponent<MeshRenderer> returned nullptr.");
            ctx.m_scene->DestroyObject(obj);
            return nullptr;
        }


        // MeshType::Fieldを設定（Perlinノイズパラメータを含む）
        Engine::MeshCreateDesc meshDesc = Engine::MeshCreateDesc::Field(
            desc.m_width, desc.m_depth, desc.m_gridSize,
            desc.m_terrainAmplitude, desc.m_terrainFrequency, desc.m_terrainOctaves, desc.m_terrainSeed);
        mr->SetMeshDesc(meshDesc);

        // マテリアル設定
        if (!ctx.m_materials) {
            Engine::Logger::Warn("FieldPrefab::SpawnWithMeshRenderer: MaterialLibrary is null, creating fallback material.");
            // フォールバック：SampleCubeMaterialを直接作成
            auto material = SampleCubeMaterial::Create(Engine::MaterialBuildContext{ ctx.m_device, ctx.m_assets });
            if (material) {
                mr->SetMaterial(material);
                Engine::Logger::Info("FieldPrefab: Using fallback SampleCubeMaterial.");
            } else {
                Engine::Logger::Error("FieldPrefab: Failed to create fallback material - rendering will not work!");
            }
        } else {
            if (desc.m_useTerrainBlend) {
                // マルチテクスチャブレンドマテリアルを使用
                Engine::Logger::Info("FieldPrefab: m_useTerrainBlend=true, requesting TerrainBlendMaterialAsset");
                auto material = ctx.m_materials->GetOrCreate<Engine::TerrainBlendMaterialAsset>();
                if (material) {
                    // Verify that the material is actually a TerrainBlendMaterial type (polymorphic check)
                    Engine::Logger::Info("[INFO] FieldPrefab: Material instance check - type=" + 
                        std::string(typeid(*material).name()) +
                        ", this=" + std::to_string(reinterpret_cast<uintptr_t>(material.get())) +
                        ", IsTerrainBlendEnabled=" + std::to_string(material->IsTerrainBlendEnabled() ? 1 : 0));
                    
                    // Check if it's actually a TerrainBlendMaterial (for polymorphic Bind)
                    auto* terrainBlendMat = dynamic_cast<Engine::TerrainBlendMaterial*>(material.get());
                    if (terrainBlendMat) {
                        Engine::Logger::Info("FieldPrefab: Material is confirmed TerrainBlendMaterial type, polymorphic Bind will work correctly");
                    } else {
                        Engine::Logger::Warn("FieldPrefab: Material is NOT TerrainBlendMaterial type (dynamic_cast failed). "
                            "Polymorphic Bind may not call TerrainBlendMaterial::Bind. Check factory implementation.");
                    }
                    
                    if (!material->IsTerrainBlendEnabled()) {
                        Engine::Logger::Error("FieldPrefab: CRITICAL - TerrainBlendMaterial was returned but IsTerrainBlendEnabled is false! "
                            "This indicates a bug in TerrainBlendMaterial::Create or MaterialLibrary caching.");
                    }
                    
                    // テクスチャをロードしてマテリアルにセット
                    Engine::TextureLoadOptions texOpt{};
                    texOpt.m_generateMipMaps = true;
                    texOpt.m_forceSRgb = true;

                    Engine::Logger::Info("FieldPrefab: Loading " + std::to_string(desc.m_activeLayerCount) + 
                        " terrain texture layers");
                    
                    for (std::uint32_t i = 0; i < desc.m_activeLayerCount && i < kMaxLayers; ++i) {
                        if (!desc.m_texturePaths[i].empty()) {
                            // Log the texture path being loaded
                            std::string narrowPath(desc.m_texturePaths[i].begin(), desc.m_texturePaths[i].end());
                            Engine::Logger::Info("FieldPrefab: Loading layer " + std::to_string(i) + 
                                " texture: " + narrowPath);
                            
                            auto tex = ctx.m_assets->LoadTexture(desc.m_texturePaths[i].c_str(), texOpt);
                            if (tex && tex->IsLoaded()) {
                                // 地形レイヤーテクスチャをマテリアルにセット
                                material->SetTerrainLayerTexture(i, tex);
                                Engine::Logger::Info("FieldPrefab: Loaded and set terrain texture layer " + 
                                    std::to_string(i) + " successfully, SRV=" + 
                                    std::to_string(reinterpret_cast<uintptr_t>(tex->GetShaderResourceView())));
                            } else {
                                Engine::Logger::Warn("FieldPrefab: Failed to load terrain texture layer " + 
                                    std::to_string(i) + " - tex is " + (tex ? "loaded=false" : "null"));
                            }
                        } else {
                            Engine::Logger::Warn("FieldPrefab: Layer " + std::to_string(i) + 
                                " has empty texture path");
                        }
                    }

                    // 地形ブレンドパラメータを設定
                    // TerrainBlendMaterial の場合は専用 API を使ってローカル状態と基底クラスを同期させる
                    if (terrainBlendMat) {
                        // 2レイヤー以上の場合、デフォルトのウェイトを (0.5, 0.5, 0.0, 0.0) に設定してブレンドを可視化
                        Engine::Vector4 effectiveWeights = desc.m_blendWeights;
                        if (desc.m_activeLayerCount >= 2) {
                            // SetTexturesのデフォルトでは全レイヤーに同じウェイト(1.0)が設定される
                            // この場合、テスト用に(0.5, 0.5, 0.0, 0.0)に置き換える
                            // 意図的に設定されたウェイトは上書きしない（例：(0.7, 0.3, 0.0, 0.0)）
                            bool isDefaultWeights = 
                                (effectiveWeights.x == effectiveWeights.y) && 
                                (effectiveWeights.x > 0.99f);  // デフォルトでは両方1.0fに設定される
                            if (isDefaultWeights) {
                                // デフォルト値の場合のみ、テスト用に均等分配
                                effectiveWeights = Engine::Vector4(0.5f, 0.5f, 0.0f, 0.0f);
                                Engine::Logger::Info("[INFO] FieldPrefab: Replacing default equal weights with test values (0.5, 0.5, 0.0, 0.0)");
                            }
                        }
                        
                        terrainBlendMat->SetBlendWeights(effectiveWeights.x, effectiveWeights.y, 
                                                          effectiveWeights.z, effectiveWeights.w);
                        terrainBlendMat->SetUVScales(desc.m_uvScales.x, desc.m_uvScales.y, 
                                                     desc.m_uvScales.z, desc.m_uvScales.w);
                        terrainBlendMat->SetActiveLayerCount(desc.m_activeLayerCount);
                        
                        for (std::uint32_t i = 0; i < kMaxLayers; ++i) {
                            terrainBlendMat->SetLayerTint(i, desc.m_layerTints[i]);
                        }
                        
                        // SetBlendWeights後の実際の値を取得してログ出力（正規化後の値）
                        const auto& actualWeights = terrainBlendMat->GetBlendWeights();
                        Engine::Logger::Info("[INFO] FieldPrefab: Set terrain blend params via TerrainBlendMaterial API - "
                            "normalized weights=(" + std::to_string(actualWeights.x) + "," + 
                            std::to_string(actualWeights.y) + "," + 
                            std::to_string(actualWeights.z) + "," + 
                            std::to_string(actualWeights.w) + ")");
                    } else {
                        // フォールバック：基底クラスのパラメータを直接設定
                        auto& blendParams = material->GetTerrainBlendParams();
                        blendParams.m_blendWeights = desc.m_blendWeights;
                        blendParams.m_uvScales = desc.m_uvScales;
                        blendParams.m_activeLayerCount = desc.m_activeLayerCount;
                        std::copy(desc.m_layerTints.begin(), desc.m_layerTints.end(), blendParams.m_layerTints);
                        
                        Engine::Logger::Warn("[WARN] FieldPrefab: Using fallback direct param setting (material is not TerrainBlendMaterial type)");
                        
                        Engine::Logger::Info("FieldPrefab: Set blend params (fallback) - activeLayerCount=" + 
                            std::to_string(blendParams.m_activeLayerCount) + 
                            ", weights=(" + std::to_string(blendParams.m_blendWeights.x) + 
                            "," + std::to_string(blendParams.m_blendWeights.y) + 
                            "," + std::to_string(blendParams.m_blendWeights.z) + 
                            "," + std::to_string(blendParams.m_blendWeights.w) + 
                            "), uvScales=(" + std::to_string(blendParams.m_uvScales.x) + 
                            "," + std::to_string(blendParams.m_uvScales.y) + 
                            "," + std::to_string(blendParams.m_uvScales.z) + 
                            "," + std::to_string(blendParams.m_uvScales.w) + ")");
                    }

                    material->EnableTexture(true);
                    mr->SetMaterial(material);

                    // Final verification
                    Engine::Logger::Info("FieldPrefab: Using TerrainBlendMaterial with " + 
                        std::to_string(desc.m_activeLayerCount) + " layers, IsTerrainBlendEnabled=" +
                        std::to_string(material->IsTerrainBlendEnabled() ? 1 : 0));
                } else {
                    Engine::Logger::Error("FieldPrefab: GetOrCreate<TerrainBlendMaterialAsset> returned nullptr! "
                        "Using fallback SampleCubeMaterial - terrain will render without blend effects (may appear solid color).");
                    auto fallbackMaterial = ctx.m_materials->GetOrCreate<SampleCubeMaterial>();
                    if (fallbackMaterial) {
                        mr->SetMaterial(fallbackMaterial);
                    } else {
                        Engine::Logger::Error("FieldPrefab: Failed to create fallback material!");
                    }
                }
            } else {
                // 単一テクスチャまたはデフォルトマテリアル
                auto material = ctx.m_materials->GetOrCreate<SampleCubeMaterial>();
                if (material) {
                    // 単一テクスチャが指定されている場合はそれを使用
                    if (!desc.m_singleTexturePath.empty()) {
                        Engine::TextureLoadOptions texOpt{};
                        texOpt.m_generateMipMaps = true;
                        texOpt.m_forceSRgb = true;
                        auto tex = ctx.m_assets->LoadTexture(desc.m_singleTexturePath.c_str(), texOpt);
                        if (tex && tex->IsLoaded()) {
                            material->SetTexture(tex);
                            material->EnableTexture(true);
                            Engine::Logger::Info("FieldPrefab: Using custom texture (path provided)");
                        } else {
                            Engine::Logger::Warn("FieldPrefab: Failed to load custom texture, using default.");
                        }
                    } else {
                        Engine::Logger::Info("FieldPrefab: Using SampleCubeMaterial with default texture.");
                    }
                    mr->SetMaterial(material);
                } else {
                    Engine::Logger::Error("FieldPrefab: Failed to create SampleCubeMaterial!");
                }
            }
        }

        // タグ設定
        obj->SetTag("Field");

        Engine::Logger::Info("FieldPrefab::SpawnWithMeshRenderer succeeded: Field created at (" +
            std::to_string(desc.m_position.x) + ", " +
            std::to_string(desc.m_position.y) + ", " +
            std::to_string(desc.m_position.z) + ") with size " +
            std::to_string(desc.m_width) + "x" + std::to_string(desc.m_depth) +
            ", grid=" + std::to_string(desc.m_gridSize));

        return obj;
    }

    Engine::GameObject* FieldPrefab::SpawnLegacyField(Engine::SceneContext& ctx, const SpawnDesc& desc) {
        // SceneのCreateObjectを使ってFieldを生成
        auto* field = ctx.m_scene->CreateObject<Field>("Field");
        if (!field) {
            Engine::Logger::Error("FieldPrefab::SpawnLegacyField failed: CreateObject returned nullptr.");
            return nullptr;
        }

        // 地形パラメータを設定
        field->SetGridSize(desc.m_legacyGridSize);
        field->SetCellSize(desc.m_cellSize);
        field->SetHeightAmplitude(desc.m_heightAmplitude);
        field->SetRoughness(desc.m_roughness);

        // Transform設定
        if (auto* tr = field->GetComponent<Engine::Transform>()) {
            tr->SetPosition(desc.m_position);
        }

        // D3Dリソースをセットアップ
        if (!field->Setup(ctx.m_device, ctx.m_renderSystem, ctx.m_assets)) {
            Engine::Logger::Error("FieldPrefab::SpawnLegacyField failed: Field::Setup failed.");
            ctx.m_scene->DestroyObject(field);
            return nullptr;
        }

        // マテリアルを設定（Legacy Fieldでも描画に必要）
        std::shared_ptr<Engine::Material> material;
        if (ctx.m_materials) {
            material = ctx.m_materials->GetOrCreate<SampleCubeMaterial>();
        } else {
            // MaterialLibraryがない場合は直接作成
            Engine::Logger::Warn("FieldPrefab::SpawnLegacyField: MaterialLibrary is null, creating fallback material.");
            material = SampleCubeMaterial::Create(Engine::MaterialBuildContext{ ctx.m_device, ctx.m_assets });
        }

        if (material) {
            field->SetMaterial(material);
            Engine::Logger::Info("FieldPrefab::SpawnLegacyField: Material set successfully.");
        } else {
            Engine::Logger::Error("FieldPrefab::SpawnLegacyField: Failed to create material - rendering may not work!");
        }

        Engine::Logger::Info("FieldPrefab::SpawnLegacyField succeeded: Field created at (" +
            std::to_string(desc.m_position.x) + ", " +
            std::to_string(desc.m_position.y) + ", " +
            std::to_string(desc.m_position.z) + ")");

        return field;
    }

} // namespace Game
