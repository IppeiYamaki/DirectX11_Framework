/// @file   AllLightPrefabs.h
/// @brief  全てのライトPrefabをまとめてインクルードするためのヘッダ
#pragma once

// 基本ライトPrefab（既存）
#include "Game/Definitions/Prefabs/LightPrefabs.h"

// 用途別ライトPrefab
#include "Game/Definitions/Prefabs/LightPrefabs/StreetLampLightPrefab.h"
#include "Game/Definitions/Prefabs/LightPrefabs/FireLightPrefab.h"
#include "Game/Definitions/Prefabs/LightPrefabs/SunLightPrefab.h"
#include "Game/Definitions/Prefabs/LightPrefabs/FlashlightLightPrefab.h"

// ライト用ビヘイビアコンポーネント
#include "Game/Definitions/Components/LightComponents/FireFlickerComponent.h"
#include "Game/Definitions/Components/LightComponents/SunCycleComponent.h"
#include "Game/Definitions/Components/LightComponents/FollowTargetLightComponent.h"
