# Sankou_00 / Sankou_01 統合ガイド

## 概要

[Sankou_00](https://github.com/IppeiYamaki/Sankou_00) と [Sankou_01](https://github.com/IppeiYamaki/Sankou_01) から DirectX11_Framework に統合可能な機能と、その統合方法を示します。

---

## Sankou_00 からの統合候補

### リポジトリ概要
- C++構造の参考リポジトリ
- ゲームフレームワークの基盤コード
- AudioSource、各種ユーティリティを含む

### 1. AudioSource（音声再生）

**統合優先度:** 中

**ソースファイル:**
- `Source/AudioSource.cpp`
- `Source/AudioSource.h`

**統合方法:**

```cpp
// Engine/Audio/AudioSource.h
#pragma once

#include <xaudio2.h>
#include <wrl/client.h>
#include <string>
#include <vector>

namespace Engine {

    /// @brief 音声ソースコンポーネント
    class AudioSource final {
    public:
        AudioSource() = default;
        ~AudioSource();

        AudioSource(const AudioSource&) = delete;
        AudioSource& operator=(const AudioSource&) = delete;

        /// @brief WAVファイルをロード
        bool Load(const std::wstring& filePath);
        void Unload();

        /// @brief 再生
        void Play();
        
        /// @brief 停止
        void Stop();
        
        /// @brief 一時停止
        void Pause();
        
        /// @brief 再開
        void Resume();

        /// @brief ループ設定
        void SetLoop(bool isLoop);
        
        /// @brief 音量設定
        void SetVolume(float volume);
        
        /// @brief ピッチ設定
        void SetPitch(float pitch);

        /// @brief 再生中かどうか
        [[nodiscard]] bool IsPlaying() const;
        
        /// @brief ロード済みかどうか
        [[nodiscard]] bool IsLoaded() const;

    private:
        IXAudio2SourceVoice* m_sourceVoice = nullptr;
        std::vector<BYTE> m_audioData;
        WAVEFORMATEX m_waveFormat{};
        bool m_isLoaded = false;
        bool m_isLoop = false;
    };

} // namespace Engine
```

**ディレクトリ構造:**
```
Engine/
  Audio/
    AudioEngine.h      // XAudio2初期化
    AudioEngine.cpp
    AudioSource.h      // 音声ソース
    AudioSource.cpp
    AudioClip.h        // 音声データ
    AudioClip.cpp
```

**Applicationへの統合:**
```cpp
// Application.h に追加
#include "Engine/Audio/AudioEngine.h"

class Application final {
private:
    std::unique_ptr<AudioEngine> m_audioEngine;
};

// Application.cpp
bool Application::Initialize(const ApplicationSettings& settings) {
    // ...
    m_audioEngine = std::make_unique<AudioEngine>();
    if (!m_audioEngine->Initialize()) {
        Logger::Error("AudioEngine Initialize failed.");
        return false;
    }
    // ...
}
```

---

### 2. Scene/State 管理パターン

**統合優先度:** 既存のSceneManagerと比較・改善

**参考ポイント:**
- ステート遷移の安全性
- リソースのロード/アンロードタイミング
- 遷移アニメーション対応

---

## Sankou_01 からの統合候補

### リポジトリ概要
- HLSL、DirectX11実装の参考リポジトリ
- ポストプロセスシェーダー集
- 高度なレンダリング技術の参考

### 1. ポストプロセスシステム（高優先度）

**利用可能なシェーダー:**

| シェーダー | 機能 | 統合優先度 |
|-----------|------|-----------|
| Bloom | 発光エフェクト | 高 |
| FXAA | アンチエイリアス | 高 |
| ToneMap | トーンマッピング | 中 |
| Blur | ブラー効果 | 中 |
| SSAO | 環境遮蔽 | 低 |
| DOF | 被写界深度 | 低 |

**統合アーキテクチャ:**

```cpp
// Engine/Graphics/PostProcess/PostProcessBase.h
#pragma once

#include <d3d11.h>
#include <wrl/client.h>

namespace Engine {

    class GraphicsDevice;

    /// @brief ポストプロセス基底クラス
    class PostProcessBase {
    public:
        PostProcessBase() = default;
        virtual ~PostProcessBase() = default;

        PostProcessBase(const PostProcessBase&) = delete;
        PostProcessBase& operator=(const PostProcessBase&) = delete;

        /// @brief 初期化
        virtual bool Initialize(GraphicsDevice& device) = 0;
        
        /// @brief 終了処理
        virtual void Finalize() = 0;
        
        /// @brief 適用
        virtual void Apply(
            ID3D11DeviceContext* context,
            ID3D11ShaderResourceView* input,
            ID3D11RenderTargetView* output
        ) = 0;

        /// @brief 有効/無効
        void Enable() { m_isEnabled = true; }
        void Disable() { m_isEnabled = false; }
        [[nodiscard]] bool IsEnabled() const { return m_isEnabled; }

    protected:
        bool m_isEnabled = true;
    };

} // namespace Engine
```

```cpp
// Engine/Graphics/PostProcess/BloomEffect.h
#pragma once

#include "Engine/Graphics/PostProcess/PostProcessBase.h"

namespace Engine {

    /// @brief Bloomエフェクト
    class BloomEffect final : public PostProcessBase {
    public:
        bool Initialize(GraphicsDevice& device) override;
        void Finalize() override;
        void Apply(
            ID3D11DeviceContext* context,
            ID3D11ShaderResourceView* input,
            ID3D11RenderTargetView* output
        ) override;

        /// @brief 輝度閾値を設定
        void SetThreshold(float threshold) { m_threshold = threshold; }
        
        /// @brief 強度を設定
        void SetIntensity(float intensity) { m_intensity = intensity; }

    private:
        // 中間バッファ
        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_brightPassTex;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_brightPassSRV;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_brightPassRTV;

        // ブラー用バッファ
        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_blurTex[2];
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_blurSRV[2];
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_blurRTV[2];

        // シェーダー
        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_extractPS;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_blurHPS;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_blurVPS;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_combinePS;

        float m_threshold = 0.8f;
        float m_intensity = 1.0f;
    };

} // namespace Engine
```

```cpp
// Engine/Graphics/PostProcess/PostProcessStack.h
#pragma once

#include <vector>
#include <memory>

#include "Engine/Graphics/PostProcess/PostProcessBase.h"

namespace Engine {

    /// @brief ポストプロセスの連鎖処理
    class PostProcessStack final {
    public:
        PostProcessStack() = default;
        ~PostProcessStack() = default;

        PostProcessStack(const PostProcessStack&) = delete;
        PostProcessStack& operator=(const PostProcessStack&) = delete;

        bool Initialize(GraphicsDevice& device, int width, int height);
        void Finalize();
        void Resize(int width, int height);

        /// @brief エフェクトを追加
        template<typename T, typename... Args>
        T* AddEffect(Args&&... args) {
            auto effect = std::make_unique<T>(std::forward<Args>(args)...);
            T* raw = effect.get();
            if (effect->Initialize(*m_device)) {
                m_effects.push_back(std::move(effect));
            }
            return raw;
        }

        /// @brief 全エフェクトを適用
        void Apply(
            ID3D11DeviceContext* context,
            ID3D11ShaderResourceView* sceneInput,
            ID3D11RenderTargetView* finalOutput
        );

    private:
        GraphicsDevice* m_device = nullptr;

        std::vector<std::unique_ptr<PostProcessBase>> m_effects;

        // ピンポンバッファ
        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pingPongTex[2];
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pingPongSRV[2];
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pingPongRTV[2];
        int m_currentBuffer = 0;
    };

} // namespace Engine
```

---

### 2. シェーダー統合手順

**Step 1: HLSLファイルのコピー**

```
Assets/
  Shaders/
    Common/
      (既存)
    PostProcess/        <- 新規
      FullscreenQuad.hlsl
      BloomExtract.hlsl
      Blur.hlsl
      BloomCombine.hlsl
      FXAA.hlsl
      ToneMap.hlsl
```

**Step 2: 共通ヘッダーの作成**

```hlsl
// Assets/Shaders/PostProcess/Common.hlsli

// フルスクリーンクアッド用の頂点構造
struct FullscreenVS_Output {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

// 共通サンプラー
SamplerState linearSampler : register(s0);
SamplerState pointSampler  : register(s1);

// 入力テクスチャ
Texture2D inputTexture : register(t0);
```

**Step 3: シェーダーの調整**

Sankou_01のシェーダーを参考に、DirectX11_Frameworkの定数バッファレイアウトに合わせて調整：

```hlsl
// Assets/Shaders/PostProcess/BloomExtract.hlsl
#include "Common.hlsli"

cbuffer BloomParams : register(b0) {
    float threshold;
    float intensity;
    float2 padding;
};

float4 main(FullscreenVS_Output input) : SV_TARGET {
    float4 color = inputTexture.Sample(linearSampler, input.texCoord);
    
    // 輝度計算
    float brightness = dot(color.rgb, float3(0.299, 0.587, 0.114));
    
    // 閾値以上の輝度を抽出
    float contribution = max(0, brightness - threshold);
    
    return float4(color.rgb * contribution * intensity, 1.0);
}
```

---

### 3. 高度なシェーダー機能（長期統合候補）

| 機能 | Sankou_01ファイル | 用途 |
|------|------------------|------|
| SSR | SSR.hlsl | スクリーンスペース反射 |
| SSAO | SSAO.hlsl | 環境遮蔽 |
| HBAO | HBAO.hlsl | 高品質AO |
| TAA | TAA.hlsl | テンポラルAA |
| ボリュームライト | VolumetricLighting_*.hlsl | 光の散乱 |
| ボリュームクラウド | VolumetricClouds.hlsl | 雲の描画 |
| レンズフレア | LensFlare.hlsl | レンズ効果 |

---

## 統合ロードマップ

### Phase 1（中期：1-2ヶ月）

1. **ポストプロセス基盤**
   - PostProcessBase, PostProcessStack
   - FullscreenQuadシェーダー
   - ピンポンバッファ

2. **基本エフェクト**
   - Bloom
   - FXAA
   - ToneMap

3. **AudioEngine**
   - XAudio2初期化
   - AudioSource基本実装

### Phase 2（長期：3-6ヶ月）

1. **高度なエフェクト**
   - SSAO
   - DOF (被写界深度)
   - MotionBlur

2. **音声機能拡張**
   - 3Dサウンド
   - BGM/SE管理
   - フェード機能

---

## コード移植時の注意点

### 1. 命名規則の適用

Sankou_00/01のコードをDirectX11_Frameworkに移植する際は、README.mdの命名規則に従う：

```cpp
// 元のコード（Sankou_00）
class audio_source {
    bool isPlaying;
    void play();
};

// 移植後（DirectX11_Framework）
class AudioSource {
    bool m_isPlaying;
    void Play();
};
```

### 2. Doxygen形式のコメント追加

```cpp
/// @brief 音声ソースコンポーネント
/// @note  XAudio2を使用した音声再生機能を提供
class AudioSource final {
public:
    /// @brief WAVファイルをロード
    /// @param filePath ファイルパス
    /// @return ロード成功時true
    bool Load(const std::wstring& filePath);
};
```

### 3. ComPtr統一

```cpp
// 手動Release()は禁止
// Before
IXAudio2* pXAudio2;
// ...
pXAudio2->Release();

// After
Microsoft::WRL::ComPtr<IXAudio2> m_xAudio2;
// 自動解放
```

### 4. エラーハンドリング

```cpp
bool AudioEngine::Initialize() {
    HRESULT hr = XAudio2Create(m_xAudio2.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr)) {
        Logger::Error("XAudio2 creation failed: {:08X}", hr);
        return false;
    }
    // ...
}
```

---

## ディレクトリ構造（統合後）

```
Engine/
  Audio/                      <- Sankou_00から
    AudioEngine.h
    AudioEngine.cpp
    AudioSource.h
    AudioSource.cpp
    AudioClip.h
    AudioClip.cpp
  Graphics/
    PostProcess/              <- Sankou_01参考
      PostProcessBase.h
      PostProcessStack.h
      PostProcessStack.cpp
      BloomEffect.h
      BloomEffect.cpp
      FXAAEffect.h
      FXAAEffect.cpp
      ToneMapEffect.h
      ToneMapEffect.cpp

Assets/
  Shaders/
    PostProcess/              <- Sankou_01から
      Common.hlsli
      FullscreenQuad.hlsl
      BloomExtract.hlsl
      BloomCombine.hlsl
      Blur.hlsl
      FXAA.hlsl
      ToneMap.hlsl
```

---

## 次のステップ

1. [ROADMAP.md](./ROADMAP.md) で全体計画を確認
2. ポストプロセス基盤から実装開始
3. Bloomを最初のエフェクトとして実装
4. 効果を確認して次のエフェクトへ
