#include "FireflyEmitterPrefab.h"

#include <random>

#include "Engine/Core/Logger.h"
#include "Engine/Scene/SceneContext.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"

#include "Engine/Graphics/RenderLayer.h"
#include "Game/Definitions/Components/FireflyParticleComponent.h"
#include "Game/Definitions/Components/ParticleRendererComponent.h"

#include "Engine/Materials/MaterialLibrary.h"
#include "Engine/Materials/ParticleMaterial.h"

namespace Game {

    Engine::GameObject* FireflyEmitterPrefab::Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc) {
        if (!ctx.m_scene || !ctx.m_renderSystem || !ctx.m_device || !ctx.m_materials) {
            Engine::Logger::Error("FireflyEmitterPrefab::Spawn failed: ctx invalid.");
            return nullptr;
        }

        // Create the emitter GameObject (parent object)
        auto* emitter = ctx.m_scene->CreateObject<Engine::GameObject>();
        if (!emitter) {
            Engine::Logger::Error("FireflyEmitterPrefab::Spawn failed: could not create emitter GameObject.");
            return nullptr;
        }

        emitter->SetName("FireflyEmitter");

        // Set emitter position
        if (auto* tr = emitter->GetComponent<Engine::Transform>()) {
            tr->SetPosition(desc.m_position);
        }

        // Random number generator
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> disX(-desc.m_spawnRange.x, desc.m_spawnRange.x);
        std::uniform_real_distribution<float> disY(0.0f, desc.m_spawnRange.y); // 水面より上
        std::uniform_real_distribution<float> disZ(-desc.m_spawnRange.z, desc.m_spawnRange.z);
        std::uniform_real_distribution<float> disSize(desc.m_minSize, desc.m_maxSize);
        std::uniform_real_distribution<float> disSpeed(0.5f, 1.5f);
        std::uniform_real_distribution<float> disFlicker(1.5f, 3.0f);
        std::uniform_real_distribution<float> disBrightness(0.1f, 0.4f);
        std::uniform_real_distribution<float> disColor(0.0f, 1.0f);

        // Create particle material
        auto particleMaterial = ctx.m_materials->GetOrCreate<Engine::ParticleMaterial>();
        if (!particleMaterial) {
            Engine::Logger::Error("FireflyEmitterPrefab::Spawn failed: could not create particle material.");
            ctx.m_scene->DestroyObject(emitter);
            return nullptr;
        }

        // Spawn particles
        for (int i = 0; i < desc.m_particleCount; ++i) {
            // Create particle GameObject
            auto* particle = ctx.m_scene->CreateObject<Engine::GameObject>();
            if (!particle) {
                Engine::Logger::Warn("FireflyEmitterPrefab: Failed to create particle " + std::to_string(i));
                continue;
            }

            particle->SetName("Firefly_" + std::to_string(i));

            // Random position within spawn range
            float x = desc.m_position.x + disX(gen);
            float y = desc.m_position.y + disY(gen);
            // Make sure Y is above water surface
            if (y < desc.m_waterSurfaceY) {
                y = desc.m_waterSurfaceY + 0.5f;
            }
            float z = desc.m_position.z + disZ(gen);

            Engine::Vector3 position(x, y, z);

            // Set transform
            if (auto* tr = particle->GetComponent<Engine::Transform>()) {
                tr->SetPosition(position);
            }

            // Add FireflyParticleComponent
            auto* fireflyComp = particle->AddComponent<FireflyParticleComponent>();
            if (fireflyComp) {
                fireflyComp->SetBasePosition(position);
                fireflyComp->SetDriftRange(Engine::Vector3(3.0f, 2.0f, 3.0f));
                fireflyComp->SetDriftSpeed(disSpeed(gen));
                fireflyComp->SetFlickerSpeed(disFlicker(gen));
                fireflyComp->SetMinBrightness(disBrightness(gen));
                fireflyComp->SetSize(disSize(gen));

                // Random color variation (mostly yellow-white, with some green tint)
                float colorVariation = disColor(gen);
                Engine::Color baseColor;
                if (colorVariation < 0.7f) {
                    // Warm yellow-white
                    baseColor = Engine::Color(1.0f, 1.0f, 0.7f + disColor(gen) * 0.3f, 1.0f);
                } else if (colorVariation < 0.9f) {
                    // Green tint
                    baseColor = Engine::Color(0.8f + disColor(gen) * 0.2f, 1.0f, 0.7f, 1.0f);
                } else {
                    // Pure white
                    baseColor = Engine::Color(1.0f, 1.0f, 1.0f, 1.0f);
                }
                fireflyComp->SetBaseColor(baseColor);
            }

            // Add ParticleRendererComponent
            auto* rendererComp = particle->AddComponent<ParticleRendererComponent>(ctx.m_device, ctx.m_renderSystem);
            if (rendererComp) {
                rendererComp->SetMaterial(particleMaterial);
                rendererComp->SetRenderLayer(Engine::RenderLayer::Transparent);
                rendererComp->SetOrderInLayer(100);
                // Use additive blending for glow effect
                rendererComp->SetRenderStateFlags(
                    Engine::kRenderStateBlendAdditive | 
                    Engine::kRenderStateDepthWriteOff
                );
            }
        }

        Engine::Logger::Info("FireflyEmitterPrefab: Created " + std::to_string(desc.m_particleCount) + 
                            " firefly particles at position (" +
                            std::to_string(desc.m_position.x) + ", " +
                            std::to_string(desc.m_position.y) + ", " +
                            std::to_string(desc.m_position.z) + ")");

        return emitter;
    }

} // namespace Game
