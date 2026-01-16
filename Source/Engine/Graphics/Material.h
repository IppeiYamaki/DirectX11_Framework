#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <memory>

#include "Engine/Graphics/ConstantBuffer.h"
#include "Engine/Graphics/ShaderConstants.h"
#include "Engine/Graphics/Shader.h"
#include "Engine/Graphics/Texture.h"

namespace Engine {

    /**
     * @brief 描画に必要な「状態」をひとまとめにしたクラス
     *
     * - VS/PS/InputLayout
     * - Material定数(b3)
     * - Texture(t0) / Sampler(s0)
     *
     * RenderSystemは「World/View/Proj/Light」を持ち、
     * Materialは「Material/Texture/Sampler/Shader」を持つ役割分担。
     */
    class Material final {
    public:
        Material() = default;
        ~Material();

        Material(const Material&) = delete;
        Material& operator=(const Material&) = delete;

        //============================================================
        // Lifecycle
        //============================================================
        bool Initialize(ID3D11Device* device);
        void Finalize();

        bool IsInitialized() const;

        //============================================================
        // Set pipeline resources
        //============================================================
        void SetVertexShader(const std::shared_ptr<VertexShader>& vs);
        void SetPixelShader(const std::shared_ptr<PixelShader>& ps);
        void SetInputLayout(const std::shared_ptr<InputLayout>& inputLayout);

        //============================================================
        // Texture/Sampler
        //============================================================
        void SetTexture(const std::shared_ptr<Texture>& texture);
        void SetSampler(ID3D11SamplerState* sampler); // 借用（nullptrなら内部デフォルト）

        //============================================================
        // Material params (b3)
        //============================================================
        MaterialParams& GetParams();
        const MaterialParams& GetParams() const;

        void EnableTexture(bool isEnabled);
        void EnableAlphaTest(bool isEnabled, float alphaCutoff = 0.5f);

        //============================================================
        // Bind
        //============================================================
        void Bind(ID3D11DeviceContext* context);


    private:
        bool CreateDefaultSampler(ID3D11Device* device);

    private:
        bool m_isInitialized = false;

        std::shared_ptr<VertexShader> m_vs;
        std::shared_ptr<PixelShader>  m_ps;
        std::shared_ptr<InputLayout>  m_inputLayout;

        std::shared_ptr<Texture>      m_texture;

        // Sampler（external優先、無ければ内部デフォルト）
        ID3D11SamplerState* m_samplerExternal = nullptr;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_defaultSampler;

        // Material constant buffer (b3)
        ConstantBuffer<MaterialCB> m_materialCb;
        MaterialCB m_materialData{};
    };

} // namespace Engine
