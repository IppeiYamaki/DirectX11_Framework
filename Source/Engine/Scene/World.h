/// @file   World.h
/// @brief  Entity群を管理し、Update/Drawを統括するクラス
#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <functional>

#include "Engine/Scene/Entity.h"

namespace Engine {

    /// @brief Entity群を管理し、Update/Drawを統括する
    /// @note  破棄の安全管理（遅延破棄）は将来的に実装予定
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

        /// @brief リセット（全Entityクリア）
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

        /// @brief  初期化済みか確認
        /// @return 初期化済みならtrue
        [[nodiscard]] bool IsInitialized() const;

        //============================================================
        // Entity API
        //============================================================

        /// @brief  新しいEntityを生成
        /// @return 生成されたEntity（Transformは自動追加）
        [[nodiscard]] Entity* CreateEntity();

        /// @brief  名前付きEntityを生成
        /// @param  name エンティティ名
        /// @return 生成されたEntity
        [[nodiscard]] Entity* CreateEntity(const std::string& name);

        /// @brief Entityを破棄（即時削除）
        /// @param entity 破棄するEntity
        /// @warning Update中に呼ぶとイテレータ破壊の危険あり
        void DestroyEntity(Entity* entity);

        /// @brief Entityを遅延破棄予約
        /// @param entity 破棄するEntity
        void DestroyEntityDeferred(Entity* entity);

        /// @brief  Entity数を取得
        /// @return Entity数
        [[nodiscard]] std::uint32_t GetEntityCount() const;

        //============================================================
        // Entity Query
        //============================================================

        /// @brief  名前でEntityを検索
        /// @param  name 検索する名前
        /// @return 見つかったEntity（存在しなければnullptr）
        [[nodiscard]] Entity* FindEntityByName(const std::string& name);

        /// @brief  IDでEntityを検索
        /// @param  id 検索するEntityId
        /// @return 見つかったEntity（存在しなければnullptr）
        [[nodiscard]] Entity* FindEntityById(EntityId id);

        /// @brief  タグを持つEntityを検索
        /// @param  tag 検索するタグ
        /// @return 見つかったEntityのベクター
        [[nodiscard]] std::vector<Entity*> FindEntitiesWithTag(const Tag& tag);

        /// @brief  特定のComponentを持つEntityを検索
        /// @tparam T 検索するComponent型
        /// @return 見つかったEntityのベクター
        template <typename T>
        [[nodiscard]] std::vector<Entity*> FindEntitiesWithComponent() {
            std::vector<Entity*> result;
            for (auto& e : m_entities) {
                if (e->GetComponent<T>() != nullptr) {
                    result.push_back(e.get());
                }
            }
            return result;
        }

        /// @brief  条件を満たすEntityを検索
        /// @param  predicate 検索条件
        /// @return 見つかったEntityのベクター
        template <typename Predicate>
        [[nodiscard]] std::vector<Entity*> FindEntitiesWhere(Predicate predicate) {
            std::vector<Entity*> result;
            for (auto& e : m_entities) {
                if (predicate(e.get())) {
                    result.push_back(e.get());
                }
            }
            return result;
        }

        /// @brief  条件を満たす最初のEntityを検索
        /// @param  predicate 検索条件
        /// @return 見つかったEntity（存在しなければnullptr）
        template <typename Predicate>
        [[nodiscard]] Entity* FindEntityWhere(Predicate predicate) {
            for (auto& e : m_entities) {
                if (predicate(e.get())) {
                    return e.get();
                }
            }
            return nullptr;
        }

    private:
        /// @brief フレーム終了時に遅延破棄を処理
        void ProcessPendingDestructions();

    private:
        bool m_isInitialized = false;                       ///< 初期化済みフラグ

        std::vector<std::unique_ptr<Entity>> m_entities;    ///< 所有Entity群
        std::vector<Entity*> m_pendingDestruction;          ///< 遅延破棄待ちリスト
    };

} // namespace Engine
