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

    void LightSystem::Update(float deltaTime) {
        (void)deltaTime;
        // 現在は追加の更新処理なし
        // 将来の拡張予定:
        // - ライトアニメーション
        // - 動的なライト位置更新
    }

    void LightSystem::ApplyToRenderSystem() {
        if (!m_isInitialized || !m_renderSystem) return;

        // 最初の有効な方向性ライトをRenderSystemに適用
        // (既存のDirectionalLight APIとの互換性維持)
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

                m_renderSystem->SetLight(shaderLight);
                break;  // 最初の有効なDirectionalLightのみを適用
            }
        }
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
