#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>

#include "Engine/Scene/Component.h"
#include "Engine/Graphics/ConstantBuffer.h"

namespace Engine {

    class Material;
    class Mesh;
    class RenderSystem;
    class PlanarReflectionSystem;
    class Texture;
    class AssetManager;

    /// @brief 水面用定数バッファ（b7）
    struct alignas(16) WaterCB final {
        float g_time = 0.0f;                    ///< 経過時間
        float g_waveSpeed = 1.0f;               ///< 波の速度
        float g_waveFrequency = 0.5f;           ///< 波の周波数
        float g_waveAmplitude = 0.1f;           ///< 波の振幅

        float g_uvScrollSpeed = 0.1f;           ///< UVスクロール速度
        float g_reflectionDistortion = 0.02f;   ///< 反射の歪み強度
        float g_pad0 = 0.0f;
        float g_pad1 = 0.0f;
    };

    /// @brief 水面コンポーネント
    /// @note  XZ平面上に配置された水面の波アニメーションと反射テクスチャの合成を担当
    /// @note  メッシュは格子状（MeshField）で、頂点単位での波変位が可能
    /// @note  法線は上向き(0,1,0)が基準、Y軸が高さ方向
    class WaterSurfaceComponent : public Component {
    public:
        /// @brief コンストラクタ
        /// @param device D3D11デバイス
        /// @param renderSystem レンダーシステム
        /// @param assetManager アセットマネージャー（反射テクスチャ読み込み用）
        explicit WaterSurfaceComponent(ID3D11Device* device, RenderSystem* renderSystem, AssetManager* assetManager);
        ~WaterSurfaceComponent() override;

        /// @brief 初期化
        void OnStart() override;

        /// @brief 更新
        /// @param deltaTime フレーム経過時間
        void Update(float deltaTime) override;

        /// @brief 描画
        void Draw() override;

        //============================================================
        // Settings
        //============================================================

        /// @brief 波の速度を設定
        void SetWaveSpeed(float speed) { m_waterData.g_waveSpeed = speed; }

        /// @brief 波の周波数を設定
        void SetWaveFrequency(float freq) { m_waterData.g_waveFrequency = freq; }

        /// @brief 波の振幅を設定
        void SetWaveAmplitude(float amp) { m_waterData.g_waveAmplitude = amp; }

        /// @brief UVスクロール速度を設定
        void SetUVScrollSpeed(float speed) { m_waterData.g_uvScrollSpeed = speed; }

        /// @brief 反射歪み強度を設定
        void SetReflectionDistortion(float dist) { m_waterData.g_reflectionDistortion = dist; }

        /// @brief 平面反射システムを設定（将来の実装用）
        /// @note  現在はSky.pngを使用しているが、この関数で実際のplanar reflection RTに切り替えられる
        void SetReflectionSystem(PlanarReflectionSystem* system) { m_reflectionSystem = system; }
        
        /// @brief 反射テクスチャを設定（Sky.pngなど静的テクスチャ用）
        void SetReflectionTexture(const std::shared_ptr<Texture>& texture) { m_reflectionTexture = texture; }

        /// @brief マテリアルを設定
        void SetMaterial(const std::shared_ptr<Material>& material) { m_material = material; }

        /// @brief メッシュを設定
        void SetMesh(const std::shared_ptr<Mesh>& mesh) { m_mesh = mesh; }

        /// @brief 水面の高さ（Y座標）を取得
        float GetWaterHeight() const;

    private:
        /// @brief 反射用サンプラーを作成
        bool CreateReflectionSampler();

    private:
        ID3D11Device* m_device = nullptr;
        RenderSystem* m_renderSystem = nullptr;
        AssetManager* m_assetManager = nullptr;
        PlanarReflectionSystem* m_reflectionSystem = nullptr;

        std::shared_ptr<Material> m_material;
        std::shared_ptr<Mesh> m_mesh;
        std::shared_ptr<Texture> m_reflectionTexture;  // Sky.pngなどの静的反射テクスチャ

        ConstantBuffer<WaterCB> m_waterCb;
        WaterCB m_waterData{};

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_reflectionSampler;

        float m_elapsedTime = 0.0f;
    };

} // namespace Engine
