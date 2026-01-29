# LightSystem 設計書

このドキュメントは、DirectX11_Frameworkに追加されたLightSystem（光源管理システム）について説明します。

---

## 目次

1. [概要](#1-概要)
2. [LightSystem](#2-lightsystem)
3. [光源クラス](#3-光源クラス)
4. [使用例](#4-使用例)
5. [シェーダー定数](#5-シェーダー定数)
6. [HDRレンダリング](#6-hdrレンダリング)
7. [ボリューメトリックライティング](#7-ボリューメトリックライティング)
8. [ファイル構成](#8-ファイル構成)

---

## 1. 概要

### 1.1 背景

従来のフレームワークでは、ライティングは以下の課題がありました：
- 単一の方向性ライトのみサポート
- シーンごとにライトを柔軟に管理する仕組みがない
- 複数種類の光源（Point、Spot）に対応していない

### 1.2 解決策

1. **LightSystem**: 光源の生成・管理をエンジン側に集約
2. **Light基底クラス**: 共通プロパティを持つ基底クラス
3. **派生クラス**: DirectionalLight, PointLight, SpotLightを個別に実装
4. **シェーダー定数拡張**: 複数光源対応のシェーダー定数を追加

---

## 2. LightSystem

### 2.1 概要

`LightSystem`は、光源の生成・管理を統括するエンジン側のシステムです。CameraSystemと同様のパターンで設計されています。

### 2.2 主な機能

- 方向性ライトの生成（`AddDirectionalLight`）
- 点光源の生成（`AddPointLight`）
- スポットライトの生成（`AddSpotLight`）
- ライトの削除（`RemoveLight`）
- RenderSystemへのライト情報適用

### 2.3 クラス設計

```cpp
class LightSystem {
public:
    bool Initialize(RenderSystem* renderSystem);
    void Finalize();

    // ライト生成
    DirectionalLightObject* AddDirectionalLight(const Vector3& direction);
    PointLightObject* AddPointLight(const Vector3& position);
    SpotLightObject* AddSpotLight(const Vector3& position, const Vector3& direction, 
                                  float innerAngle, float outerAngle);

    // ライト管理
    void RemoveLight(Light* light);
    void ClearLights();
    std::size_t GetLightCount() const;

    // 更新・適用
    void Update(float deltaTime);
    void ApplyToRenderSystem();
};
```

---

## 3. 光源クラス

### 3.1 Light（基底クラス）

全てのライトに共通のプロパティを持つ基底クラスです。

```cpp
class Light {
public:
    virtual LightType GetLightType() const = 0;

    void SetColor(const Vector3& color);
    Vector3 GetColor() const;

    void SetIntensity(float intensity);
    float GetIntensity() const;

    void Enable();
    void Disable();
    bool IsEnabled() const;

    void SetCastShadow(bool castShadow);
    bool IsCastShadow() const;

    void SetAmbient(const Vector3& ambient);
    Vector3 GetAmbient() const;

protected:
    Vector3 m_color{1.0f, 1.0f, 1.0f};
    float m_intensity = 1.0f;
    bool m_isEnabled = true;
    bool m_castShadow = false;
    Vector3 m_ambient{0.1f, 0.1f, 0.1f};
};
```

### 3.2 DirectionalLightObject（方向性ライト）

太陽光のような無限遠からの光を表現します。

```cpp
class DirectionalLightObject : public Light {
public:
    LightType GetLightType() const override { return LightType::Directional; }

    void SetDirection(const Vector3& direction);
    Vector3 GetDirection() const;

private:
    Vector3 m_direction{0.0f, -1.0f, 0.0f};
};
```

### 3.3 PointLightObject（点光源）

全方向に放射される光を表現します。

```cpp
class PointLightObject : public Light {
public:
    LightType GetLightType() const override { return LightType::Point; }

    void SetPosition(const Vector3& position);
    Vector3 GetPosition() const;

    void SetRange(float range);
    float GetRange() const;

    void SetAttenuation(float constant, float linear, float quadratic);
    void GetAttenuation(float& constant, float& linear, float& quadratic) const;

protected:
    Vector3 m_position{0.0f, 0.0f, 0.0f};
    float m_range = 10.0f;
    float m_constantAttenuation = 1.0f;
    float m_linearAttenuation = 0.09f;
    float m_quadraticAttenuation = 0.032f;
};
```

### 3.4 SpotLightObject（スポットライト）

円錐状に放射される光を表現します。PointLightObjectを継承しています。

```cpp
class SpotLightObject : public PointLightObject {
public:
    LightType GetLightType() const override { return LightType::Spot; }

    void SetDirection(const Vector3& direction);
    Vector3 GetDirection() const;

    void SetConeAngles(float innerAngle, float outerAngle);
    void GetConeAngles(float& innerAngle, float& outerAngle) const;

    float GetInnerCosAngle() const;
    float GetOuterCosAngle() const;

private:
    Vector3 m_direction{0.0f, -1.0f, 0.0f};
    float m_innerAngle = 30.0f;
    float m_outerAngle = 45.0f;
};
```

---

## 4. 使用例

### 4.1 SceneでのLightSystem使用

```cpp
void GameScene::BuildScene(SceneContext& ctx) {
    if (ctx.m_lightSystem) {
        // Directional Light を追加
        auto* dirLight = ctx.m_lightSystem->AddDirectionalLight({1.0f, -1.0f, 1.0f});
        dirLight->SetColor({1.0f, 0.9f, 0.8f});
        dirLight->SetIntensity(1.5f);

        // Point Light を追加
        auto* pointLight = ctx.m_lightSystem->AddPointLight({5.0f, 10.0f, 5.0f});
        pointLight->SetColor({0.8f, 0.8f, 1.0f});
        pointLight->SetIntensity(2.0f);
        pointLight->SetRange(20.0f);

        // Spot Light を追加
        auto* spotLight = ctx.m_lightSystem->AddSpotLight(
            {0.0f, 5.0f, -5.0f},  // 位置
            {0.0f, -1.0f, 0.0f},  // 方向
            30.0f,                 // 内側角度
            40.0f                  // 外側角度
        );
        spotLight->SetColor({1.0f, 1.0f, 0.9f});
        spotLight->SetIntensity(3.5f);
    }
}
```

### 4.2 ライトの動的更新

```cpp
void GameScene::Update(SceneContext& ctx, float deltaTime) {
    // ライトの位置を動的に更新
    auto pointLights = ctx.m_lightSystem->GetPointLights();
    for (auto* light : pointLights) {
        Vector3 pos = light->GetPosition();
        pos.y = 5.0f + std::sin(m_elapsedTime) * 2.0f;
        light->SetPosition(pos);
    }
}
```

---

## 5. シェーダー定数

### 5.1 拡張ライト定数バッファ

複数ライト対応のためのシェーダー定数が追加されました。

```cpp
// 最大ライト数
static constexpr uint32_t kMaxDirectionalLights = 4;
static constexpr uint32_t kMaxPointLights = 16;
static constexpr uint32_t kMaxSpotLights = 16;

struct ExtendedLightCB {
    DirectionalLight g_directionalLights[kMaxDirectionalLights];
    PointLight g_pointLights[kMaxPointLights];
    SpotLight g_spotLights[kMaxSpotLights];

    uint32_t g_directionalLightCount;
    uint32_t g_pointLightCount;
    uint32_t g_spotLightCount;
    uint32_t g_pad0;
};
```

### 5.2 PointLight構造体

```cpp
struct PointLight {
    float m_positionX, m_positionY, m_positionZ;
    float m_range;

    Vector4 m_diffuse;

    float m_constantAtten;
    float m_linearAtten;
    float m_quadraticAtten;
    uint32_t m_flags;
};
```

### 5.3 SpotLight構造体

```cpp
struct SpotLight {
    float m_positionX, m_positionY, m_positionZ;
    float m_range;

    float m_directionX, m_directionY, m_directionZ;
    float m_innerCosAngle;

    Vector4 m_diffuse;

    float m_outerCosAngle;
    float m_constantAtten;
    float m_linearAtten;
    float m_quadraticAtten;

    uint32_t m_flags;
    // ... padding
};
```

---

## 6. HDRレンダリング

### 6.1 HDR設定定数バッファ

HDRレンダリングのための設定パラメータが追加されました。

```cpp
struct HDRSettingsCB {
    float g_exposure = 1.0f;         // 露出
    float g_gamma = 2.2f;            // ガンマ値
    float g_displayNits = 80.0f;     // ディスプレイ基準輝度（nits）
    float g_paperWhiteNits = 200.0f; // 紙白輝度（nits）

    float g_maxLuminance = 1000.0f;  // 最大輝度
    float g_minLuminance = 0.001f;   // 最小輝度
    uint32_t g_tonemapMode = 0;      // トーンマップモード
    uint32_t g_pad0;
};
```

### 6.2 トーンマップモード

- `0`: なし（リニア）
- `1`: Reinhard
- `2`: ACES Filmic

---

## 7. ボリューメトリックライティング

### 7.1 設定定数バッファ

ボリューメトリックライティング（光の散乱効果）のための設定です。

```cpp
struct VolumetricLightingCB {
    float g_scatteringIntensity = 0.5f;  // 散乱強度
    float g_fogDensity = 0.01f;          // フォグ密度
    uint32_t g_sampleCount = 64;         // サンプル数
    uint32_t g_enableVolumetric = 0;     // 有効フラグ

    float g_absorptionR, g_absorptionG, g_absorptionB; // 吸収係数
    float g_anisotropy = 0.5f;           // 異方性係数（-1〜1）
};
```

### 7.2 実装予定

ボリューメトリックライティングの完全な実装には以下が必要です：
- シャドウマップの生成
- 深度テクスチャの活用
- レイマーチングシェーダー

---

## 8. ファイル構成

```
Source/
├── Engine/
│   ├── Graphics/
│   │   └── ShaderConstants.h    （更新）PointLight/SpotLight/HDR追加
│   │
│   └── Scene/
│       ├── Light.h               光源クラスヘッダ
│       ├── Light.cpp             光源クラス実装
│       ├── LightSystem.h         光源管理システムヘッダ
│       ├── LightSystem.cpp       光源管理システム実装
│       └── SceneContext.h       （更新）LightSystem追加
│
├── Game/
│   ├── GameMain.h               （更新）LightSystem管理追加
│   ├── GameMain.cpp             （更新）初期化・更新追加
│   └── Scenes/
│       └── GameScene.cpp        （更新）LightSystem使用例
│
└── Docs/
    └── LIGHTSYSTEM_DESIGN.md     本ドキュメント
```

---

## 9. 依存関係

```
GameMain
    │
    ├── LightSystem ──────────────────┐
    │       │                         │
    │       ├── DirectionalLightObject│
    │       ├── PointLightObject      │
    │       └── SpotLightObject       │
    │                                 │
SceneContext ─────────────────────────┘
    │
    └── (各Sceneから参照)
```

---

## 10. 今後の拡張予定

- [ ] 複数ライトの同時シェーダー対応
- [ ] シャドウマップ生成
- [ ] ボリューメトリックライティングシェーダー
- [ ] ライトプローブシステム
- [ ] ライトマップベイク機能
- [ ] DirectXTexを活用したHDRテクスチャ読み込み

---

## 11. DirectXTexライブラリ活用

### 11.1 現在のサポート

Textureクラスで以下のフォーマットをサポートしています：
- DDS形式
- WIC対応形式（PNG, JPG, BMP等）

### 11.2 将来の拡張

DirectXTexを活用して以下の機能を追加予定：
- HDR形式テクスチャの読み込み
- ライトマップテクスチャの生成・保存
- シャドウマップのキャッシュ
