/// @file   LightSystem.cpp
/// @brief  光源の生成・管理を統括するクラス実装
#include "LightSystem.h"

#include <algorithm>

#include "Engine/Core/Logger.h"
#include "Engine/Graphics/RenderSystem.h"
#include "Engine/Scene/Light.h"

namespace Engine {

    LightSystem::~LightSystem() {
        Finalize();
    }

    //============================================================
    // Lifecycle
    //============================================================

    bool LightSystem::Initialize(RenderSystem* renderSystem) {
        if (m_isInitialized) return true;

        if (!renderSystem) {
            Logger::Error("LightSystem::Initialize failed: invalid parameters.");
            return false;
        }

        m_renderSystem = renderSystem;
        m_lights.clear();

        m_isInitialized = true;
        Logger::Info("LightSystem initialized.");
        return true;
    }

    void LightSystem::Finalize() {
        if (!m_isInitialized) return;

        ClearLights();
        m_renderSystem = nullptr;

        m_isInitialized = false;
        Logger::Info("LightSystem finalized.");
    }

    //============================================================
    // Light Management - Directional
    //============================================================

    DirectionalLightObject* LightSystem::AddDirectionalLight(const Vector3& direction) {
        if (!m_isInitialized) {
            Logger::Error("LightSystem::AddDirectionalLight failed: not initialized.");
            return nullptr;
        }

        auto light = std::make_unique<DirectionalLightObject>();
        light->SetDirection(direction);

        DirectionalLightObject* raw = light.get();
        m_lights.emplace_back(std::move(light));

        Logger::Info("Added DirectionalLight.");
        return raw;
    }

    //============================================================
    // Light Management - Point
    //============================================================

    PointLightObject* LightSystem::AddPointLight(const Vector3& position) {
        if (!m_isInitialized) {
            Logger::Error("LightSystem::AddPointLight failed: not initialized.");
            return nullptr;
        }

        auto light = std::make_unique<PointLightObject>();
        light->SetPosition(position);

        PointLightObject* raw = light.get();
        m_lights.emplace_back(std::move(light));

        Logger::Info("Added PointLight.");
        return raw;
    }

    //============================================================
    // Light Management - Spot
    //============================================================

    SpotLightObject* LightSystem::AddSpotLight(const Vector3& position, const Vector3& direction, float innerAngle, float outerAngle) {
        if (!m_isInitialized) {
            Logger::Error("LightSystem::AddSpotLight failed: not initialized.");
            return nullptr;
        }

        auto light = std::make_unique<SpotLightObject>();
        light->SetPosition(position);
        light->SetDirection(direction);
        light->SetConeAngles(innerAngle, outerAngle);

        SpotLightObject* raw = light.get();
        m_lights.emplace_back(std::move(light));

        Logger::Info("Added SpotLight.");
        return raw;
    }

    //============================================================
    // Light Management - Common
    //============================================================

    void LightSystem::RemoveLight(Light* light) {
        if (!m_isInitialized || !light) return;

        auto it = std::find_if(m_lights.begin(), m_lights.end(),
            [light](const std::unique_ptr<Light>& ptr) {
                return ptr.get() == light;
            });

        if (it != m_lights.end()) {
            m_lights.erase(it);
            Logger::Info("Removed Light.");
        }
    }

    void LightSystem::ClearLights() {
        m_lights.clear();
    }

    std::size_t LightSystem::GetLightCount() const {
        return m_lights.size();
    }

    std::size_t LightSystem::GetDirectionalLightCount() const {
        std::size_t count = 0;
        for (const auto& light : m_lights) {
            if (light->GetLightType() == LightType::Directional) {
                ++count;
            }
        }
        return count;
    }

    std::size_t LightSystem::GetPointLightCount() const {
        std::size_t count = 0;
        for (const auto& light : m_lights) {
            if (light->GetLightType() == LightType::Point) {
                ++count;
            }
        }
        return count;
    }

    std::size_t LightSystem::GetSpotLightCount() const {
        std::size_t count = 0;
        for (const auto& light : m_lights) {
            if (light->GetLightType() == LightType::Spot) {
                ++count;
            }
        }
        return count;
    }

    //============================================================
    // Frame
    //============================================================

    void LightSystem::Update([[maybe_unused]] float deltaTime) {
        // 現在は追加の更新処理なし
        // 将来の拡張予定:
        // - ライトアニメーション
        // - 動的なライト位置更新
    }

    void LightSystem::ApplyToRenderSystem() {
        if (!m_isInitialized || !m_renderSystem) return;

        // 拡張ライトデータを構築
        ExtendedLightCB extendedLightData{};
        std::uint32_t dirLightCount = 0;
        std::uint32_t pointLightCount = 0;
        std::uint32_t spotLightCount = 0;

        // 最初の有効な方向性ライトをRenderSystemに適用（レガシーAPI互換）
        bool firstDirLightApplied = false;

        for (const auto& light : m_lights) {
            if (!light->IsEnabled()) continue;

            if (light->GetLightType() == LightType::Directional) {
                auto* dirLight = static_cast<DirectionalLightObject*>(light.get());

                DirectionalLight shaderLight{};
                Vector3 dir = dirLight->GetDirection();
                shaderLight.m_directionX = dir.x;
                shaderLight.m_directionY = dir.y;
                shaderLight.m_directionZ = dir.z;
                shaderLight.m_flags = kLightFlagEnabled;

                Vector3 color = dirLight->GetColor();
                float intensity = dirLight->GetIntensity();
                shaderLight.m_diffuse = Vector4(color.x * intensity, color.y * intensity, color.z * intensity, 1.0f);

                Vector3 ambient = dirLight->GetAmbient();
                shaderLight.m_ambient = Vector4(ambient.x, ambient.y, ambient.z, 1.0f);

                // レガシーAPI: 最初の方向性ライトを設定
                if (!firstDirLightApplied) {
                    m_renderSystem->SetLight(shaderLight);
                    firstDirLightApplied = true;
                }

                // 拡張ライト配列に追加
                if (dirLightCount < kMaxDirectionalLights) {
                    extendedLightData.g_directionalLights[dirLightCount] = shaderLight;
                    ++dirLightCount;
                }
            }
            else if (light->GetLightType() == LightType::Point) {
                if (pointLightCount >= kMaxPointLights) continue;

                auto* pointLight = static_cast<PointLightObject*>(light.get());

                PointLight shaderLight{};
                Vector3 pos = pointLight->GetPosition();
                shaderLight.m_positionX = pos.x;
                shaderLight.m_positionY = pos.y;
                shaderLight.m_positionZ = pos.z;
                shaderLight.m_range = pointLight->GetRange();

                Vector3 color = pointLight->GetColor();
                float intensity = pointLight->GetIntensity();
                shaderLight.m_diffuse = Vector4(color.x * intensity, color.y * intensity, color.z * intensity, intensity);

                float constant, linear, quadratic;
                pointLight->GetAttenuation(constant, linear, quadratic);
                shaderLight.m_constantAtten = constant;
                shaderLight.m_linearAtten = linear;
                shaderLight.m_quadraticAtten = quadratic;
                shaderLight.m_flags = kLightFlagEnabled;

                extendedLightData.g_pointLights[pointLightCount] = shaderLight;
                ++pointLightCount;
            }
            else if (light->GetLightType() == LightType::Spot) {
                if (spotLightCount >= kMaxSpotLights) continue;

                auto* spotLight = static_cast<SpotLightObject*>(light.get());

                SpotLight shaderLight{};
                Vector3 pos = spotLight->GetPosition();
                shaderLight.m_positionX = pos.x;
                shaderLight.m_positionY = pos.y;
                shaderLight.m_positionZ = pos.z;
                shaderLight.m_range = spotLight->GetRange();

                Vector3 dir = spotLight->GetDirection();
                shaderLight.m_directionX = dir.x;
                shaderLight.m_directionY = dir.y;
                shaderLight.m_directionZ = dir.z;

                shaderLight.m_innerCosAngle = spotLight->GetInnerCosAngle();
                shaderLight.m_outerCosAngle = spotLight->GetOuterCosAngle();

                Vector3 color = spotLight->GetColor();
                float intensity = spotLight->GetIntensity();
                shaderLight.m_diffuse = Vector4(color.x * intensity, color.y * intensity, color.z * intensity, intensity);

                float constant, linear, quadratic;
                spotLight->GetAttenuation(constant, linear, quadratic);
                shaderLight.m_constantAtten = constant;
                shaderLight.m_linearAtten = linear;
                shaderLight.m_quadraticAtten = quadratic;
                shaderLight.m_flags = kLightFlagEnabled;

                extendedLightData.g_spotLights[spotLightCount] = shaderLight;
                ++spotLightCount;
            }
        }

        // ライト数を設定
        extendedLightData.g_directionalLightCount = dirLightCount;
        extendedLightData.g_pointLightCount = pointLightCount;
        extendedLightData.g_spotLightCount = spotLightCount;

        // 拡張ライトデータをRenderSystemに設定
        m_renderSystem->SetExtendedLights(extendedLightData);

        // 拡張ライティングを有効化（ポイントライトやスポットライトがある場合）
        bool hasExtendedLights = (pointLightCount > 0) || (spotLightCount > 0) || (dirLightCount > 0);
        m_renderSystem->SetExtendedLightingEnabled(hasExtendedLights);
    }

    //============================================================
    // Query
    //============================================================

    std::vector<DirectionalLightObject*> LightSystem::GetDirectionalLights() const {
        std::vector<DirectionalLightObject*> result;
        for (const auto& light : m_lights) {
            if (light->GetLightType() == LightType::Directional) {
                result.push_back(static_cast<DirectionalLightObject*>(light.get()));
            }
        }
        return result;
    }

    std::vector<PointLightObject*> LightSystem::GetPointLights() const {
        std::vector<PointLightObject*> result;
        for (const auto& light : m_lights) {
            if (light->GetLightType() == LightType::Point) {
                result.push_back(static_cast<PointLightObject*>(light.get()));
            }
        }
        return result;
    }

    std::vector<SpotLightObject*> LightSystem::GetSpotLights() const {
        std::vector<SpotLightObject*> result;
        for (const auto& light : m_lights) {
            if (light->GetLightType() == LightType::Spot) {
                result.push_back(static_cast<SpotLightObject*>(light.get()));
            }
        }
        return result;
    }

    bool LightSystem::IsInitialized() const {
        return m_isInitialized;
    }

} // namespace Engine
