/// @file   World.h
/// @brief  Scene群を統括するWorldクラス
#pragma once

#include <memory>

#include "Engine/Scene/Scene.h"

#include "Engine/Physics/Physics.h"

namespace Engine {

    //class PhysicsSystem;

    /// @brief Scene群を統括するWorld
    /// @note  Application側でUpdate/Draw呼び出しを統括する
    class World final {
    public:
        World() = default;
        ~World();

        World(const World&) = delete;
        World& operator=(const World&) = delete;

        //============================================================
        // Lifecycle
        //============================================================

        /// @brief  初期化
        /// @return 成功ならtrue
        [[nodiscard]] bool Initialize();

        /// @brief 終了処理
        void Finalize();

        /// @brief リセット
        void Reset();

        //============================================================
        // Frame
        //============================================================

        /// @brief 毎フレーム更新
        /// @param deltaTime フレーム経過時間
        void Update(float deltaTime);

        /// @brief 毎フレーム遅延更新
        /// @param deltaTime フレーム経過時間
        void LateUpdate(float deltaTime);

        /// @brief 毎フレーム描画
        void Draw();

        //============================================================
        // Accessors
        //============================================================

        /// @brief  初期化済みか確認
        /// @return 初期化済みならtrue
        [[nodiscard]] bool IsInitialized() const;

        /// @brief  Sceneを取得
        /// @return Sceneへのポインタ
        [[nodiscard]] Scene* GetScene();

        /// @brief  Sceneを取得（const版）
        /// @return Sceneへのconstポインタ
        [[nodiscard]] const Scene* GetScene() const;

        /// @brief  PhysicsSystemを取得
        /// @return PhysicsSystemへのポインタ
        [[nodiscard]] PhysicsSystem* GetPhysicsSystem();

        /// @brief  PhysicsSystemを取得（const版）
        /// @return PhysicsSystemへのconstポインタ
        [[nodiscard]] const PhysicsSystem* GetPhysicsSystem() const;

    private:
        bool m_isInitialized = false;                       ///< 初期化済みフラグ
        std::unique_ptr<Scene> m_scene;                     ///< Scene（所有）
        std::unique_ptr<PhysicsSystem> m_physicsSystem;     ///< PhysicsSystem（所有）
    };

} // namespace Engine
