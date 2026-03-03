/// @file   PlayerPrefab_Duck.h
/// @brief  アヒル（Duck）モデルを使用したプレイヤーPrefab
#pragma once

#include "Engine/Math/Vector3.h"
#include <string>

namespace Engine {
    class GameObject;
    struct SceneContext;
}

namespace Game {

    class PlayerObject;

    /// @brief アヒル（Duck）モデルを使用したプレイヤーPrefab
    /// @note  Characters/Duck.obj をロードしてプレイヤーキャラクターとして配置
    /// @example auto* player = ctx.Spawn<PlayerPrefab_Duck>(position, scale);
    class PlayerPrefab_Duck final {
    public:
        /// @brief 生成パラメータ
        struct SpawnDesc final {
            Engine::Vector3 m_position{ 0.0f, 0.0f, 0.0f };  ///< 配置位置
            float m_uniformScale = 1.0f;                      ///< 均一スケール
            float m_moveSpeed = 5.0f;                         ///< 移動速度
            float m_health = 100.0f;                          ///< HP
            std::string m_name = "Player(Duck)";              ///< オブジェクト名

            SpawnDesc() = default;

            /// @brief 位置指定コンストラクタ
            explicit SpawnDesc(const Engine::Vector3& position)
                : m_position(position) {
            }

            /// @brief 位置・スケール指定コンストラクタ
            SpawnDesc(const Engine::Vector3& position, float uniformScale)
                : m_position(position)
                , m_uniformScale(uniformScale) {
            }

            /// @brief フル指定コンストラクタ
            SpawnDesc(const Engine::Vector3& position,
                      float uniformScale,
                      float moveSpeed,
                      float health = 100.0f,
                      const std::string& name = "Player(Duck)")
                : m_position(position)
                , m_uniformScale(uniformScale)
                , m_moveSpeed(moveSpeed)
                , m_health(health)
                , m_name(name) {
            }
        };

        /// @brief アヒルプレイヤーをスポーン
        /// @param ctx SceneContext
        /// @param desc 生成パラメータ
        /// @return 生成されたGameObject（PlayerObject派生）
        static Engine::GameObject* Spawn(Engine::SceneContext& ctx, const SpawnDesc& desc);

        //============================================================
        // モデルパス定数
        //============================================================
        /// @brief Duckモデルのパス
        static constexpr const wchar_t* kDuckModelPath = L"Assets/Models/Characters/Duck/Duck.obj";

        /// @brief フォールバック用キューブモデル（モデルロード失敗時）
        static constexpr const wchar_t* kFallbackModelPath = nullptr;  // MeshType::Cubeを使用

    private:
        /// @brief モデルをロードしてRendererをセットアップ
        /// @param ctx SceneContext
        /// @param obj ターゲットGameObject
        /// @return 成功した場合true
        static bool SetupModelRenderer(Engine::SceneContext& ctx, Engine::GameObject* obj);
    };

} // namespace Game
