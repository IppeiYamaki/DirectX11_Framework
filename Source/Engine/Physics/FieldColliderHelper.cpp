/// @file   FieldColliderHelper.cpp
/// @brief  Field地形用のコライダーヘルパーコンポーネント実装
#include "FieldColliderHelper.h"

#include "Engine/Math/PerlinNoise.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Components/Transform.h"
#include "Engine/Core/Logger.h"

namespace Engine {

    FieldColliderHelper::FieldColliderHelper()
        : m_width(100.0f)
        , m_depth(100.0f)
        , m_amplitude(10.0f)
        , m_frequency(0.05f)
        , m_octaves(4)
        , m_seed(0)
        , m_perlinNoise(nullptr)
        , m_worldOffset() {
    }

    FieldColliderHelper::~FieldColliderHelper() = default;

    //============================================================
    // Lifecycle
    //============================================================

    void FieldColliderHelper::OnAwake() {
        // Transformからワールドオフセットを取得
        auto* owner = GetOwner();
        if (owner) {
            auto* transform = owner->GetTransform();
            if (transform) {
                m_worldOffset = transform->GetWorldPosition();
            }
        }
    }

    //============================================================
    // 設定
    //============================================================

    void FieldColliderHelper::SetFieldParams(
        float width, float depth,
        float amplitude, float frequency,
        int octaves, std::uint32_t seed
    ) {
        m_width = width;
        m_depth = depth;
        m_amplitude = amplitude;
        m_frequency = frequency;
        m_octaves = octaves;
        m_seed = seed;

        // Perlin Noiseを初期化
        m_perlinNoise = std::make_unique<PerlinNoise>(seed);

        Logger::Info("FieldColliderHelper: Params set - width=" + std::to_string(width) +
                    ", depth=" + std::to_string(depth) +
                    ", amplitude=" + std::to_string(amplitude) +
                    ", frequency=" + std::to_string(frequency));
    }

    float FieldColliderHelper::GetHeightAt(float x, float z) const {
        if (!m_perlinNoise) {
            return 0.0f;
        }

        // ワールド座標をノイズ座標に変換
        // フィールドの中心を原点とする
        float halfWidth = m_width * 0.5f;
        float halfDepth = m_depth * 0.5f;

        // ワールドオフセットを考慮
        float localX = x - m_worldOffset.x;
        float localZ = z - m_worldOffset.z;

        // 範囲外チェック
        if (localX < -halfWidth || localX > halfWidth ||
            localZ < -halfDepth || localZ > halfDepth) {
            return m_worldOffset.y; // フィールド外はデフォルト高さ
        }

        // ノイズ座標に変換してサンプリング
        float noiseX = localX * m_frequency;
        float noiseZ = localZ * m_frequency;

        float height = SampleFractalNoise(noiseX, noiseZ);
        return m_worldOffset.y + height * m_amplitude;
    }

    Vector3 FieldColliderHelper::GetNormalAt(float x, float z) const {
        // 中心差分で法線を計算
        float delta = 0.1f;
        float hL = GetHeightAt(x - delta, z);
        float hR = GetHeightAt(x + delta, z);
        float hD = GetHeightAt(x, z - delta);
        float hU = GetHeightAt(x, z + delta);

        Vector3 normal(
            (hL - hR) / (2.0f * delta),
            1.0f,
            (hD - hU) / (2.0f * delta)
        );

        return normal.Normalized();
    }

    //============================================================
    // HeightfieldCollider連携
    //============================================================

    void FieldColliderHelper::SetupHeightfieldCollider(HeightfieldColliderComponent* collider) {
        if (!collider) return;

        // 高さ関数を設定
        collider->SetHeightFunction([this](float x, float z) {
            return this->GetHeightAt(x, z);
        });

        // 範囲を設定
        float halfWidth = m_width * 0.5f;
        float halfDepth = m_depth * 0.5f;
        collider->SetBounds(
            m_worldOffset.x - halfWidth,
            m_worldOffset.x + halfWidth,
            m_worldOffset.z - halfDepth,
            m_worldOffset.z + halfDepth
        );

        collider->SetDefaultHeight(m_worldOffset.y);

        Logger::Info("FieldColliderHelper: HeightfieldCollider configured.");
    }

    //============================================================
    // Private
    //============================================================

    float FieldColliderHelper::SampleFractalNoise(float x, float z) const {
        if (!m_perlinNoise) {
            return 0.0f;
        }

        // fractal関数でフラクタルノイズを生成
        double noiseValue = m_perlinNoise->fractal(
            static_cast<double>(x),
            static_cast<double>(z),
            m_octaves,
            0.5,  // persistence
            2.0   // lacunarity
        );

        // [0, 1] -> [-1, 1] -> [0, 1] (中央を0とする)
        // fractalは既に[0,1]範囲なので、[-0.5, 0.5]にシフトして使用
        return static_cast<float>(noiseValue - 0.5) * 2.0f;
    }

} // namespace Engine
