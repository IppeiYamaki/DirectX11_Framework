/// @file   EntityId.h
/// @brief  GameObjectの一意識別子とタグシステム
#pragma once

#include <cstdint>
#include <string>
#include <set>
#include <functional>

namespace Engine {

    //============================================================
    // EntityId（GameObjectの識別子として使用）
    //============================================================

    /// @brief GameObjectの一意識別子
    struct EntityId {
        std::uint64_t m_value = 0;

        /// @brief 等価比較演算子
        constexpr bool operator==(const EntityId& other) const = default;

        /// @brief 三方比較演算子
        constexpr auto operator<=>(const EntityId& other) const = default;

        /// @brief 有効なIDか確認
        /// @return 有効ならtrue
        [[nodiscard]] constexpr bool IsValid() const noexcept {
            return m_value != 0;
        }

        /// @brief 無効なIDか確認
        /// @return 無効ならtrue
        [[nodiscard]] constexpr bool IsInvalid() const noexcept {
            return m_value == 0;
        }

        /// @brief 新しい一意のIDを生成
        /// @return 新しいEntityId
        [[nodiscard]] static EntityId Generate() {
            static std::uint64_t s_nextId = 1;
            return EntityId{ s_nextId++ };
        }

        /// @brief 無効なIDを返す
        /// @return 無効なEntityId
        [[nodiscard]] static constexpr EntityId Invalid() noexcept {
            return EntityId{ 0 };
        }
    };

    //============================================================
    // EntityIdHash (unordered_map/set用)
    //============================================================

    /// @brief EntityId用のハッシュ関数オブジェクト
    struct EntityIdHash {
        std::size_t operator()(const EntityId& id) const noexcept {
            return std::hash<std::uint64_t>{}(id.m_value);
        }
    };

    //============================================================
    // Tag
    //============================================================

    /// @brief タグ型（文字列ベース）
    using Tag = std::string;

    /// @brief タグセット
    using TagSet = std::set<Tag>;

    //============================================================
    // EntityIdentity（Name + Tags を保持）
    // Note: GameObjectはこのクラスを直接使用せず、独自にメンバを保持
    //============================================================

    /// @brief オブジェクトの識別情報を保持するクラス（レガシー、参考用）
    class EntityIdentity final {
    public:
        EntityIdentity() : m_id(EntityId::Generate()) {}
        ~EntityIdentity() = default;

        EntityIdentity(const EntityIdentity&) = delete;
        EntityIdentity& operator=(const EntityIdentity&) = delete;

        //============================================================
        // ID
        //============================================================

        /// @brief オブジェクトIDを取得
        /// @return EntityId
        [[nodiscard]] EntityId GetId() const noexcept {
            return m_id;
        }

        //============================================================
        // Name
        //============================================================

        /// @brief 名前を設定
        /// @param name 名前
        void SetName(const std::string& name) {
            m_name = name;
        }

        /// @brief 名前を設定（ムーブ版）
        /// @param name 名前
        void SetName(std::string&& name) {
            m_name = std::move(name);
        }

        /// @brief 名前を取得
        /// @return 名前への参照
        [[nodiscard]] const std::string& GetName() const noexcept {
            return m_name;
        }

        /// @brief 名前が設定されているか確認
        /// @return 名前があればtrue
        [[nodiscard]] bool HasName() const noexcept {
            return !m_name.empty();
        }

        //============================================================
        // Tags
        //============================================================

        /// @brief タグを追加
        /// @param tag 追加するタグ
        void AddTag(const Tag& tag) {
            m_tags.insert(tag);
        }

        /// @brief タグを削除
        /// @param tag 削除するタグ
        void RemoveTag(const Tag& tag) {
            m_tags.erase(tag);
        }

        /// @brief 全タグを削除
        void ClearTags() {
            m_tags.clear();
        }

        /// @brief タグを持っているか確認
        /// @param tag チェックするタグ
        /// @return タグを持っていればtrue
        [[nodiscard]] bool HasTag(const Tag& tag) const {
            return m_tags.contains(tag);
        }

        /// @brief 指定した全てのタグを持っているか確認
        /// @param tags チェックするタグセット
        /// @return 全てのタグを持っていればtrue
        [[nodiscard]] bool HasAllTags(const TagSet& tags) const {
            for (const auto& tag : tags) {
                if (!m_tags.contains(tag)) {
                    return false;
                }
            }
            return true;
        }

        /// @brief 指定したいずれかのタグを持っているか確認
        /// @param tags チェックするタグセット
        /// @return いずれかのタグを持っていればtrue
        [[nodiscard]] bool HasAnyTag(const TagSet& tags) const {
            for (const auto& tag : tags) {
                if (m_tags.contains(tag)) {
                    return true;
                }
            }
            return false;
        }

        /// @brief タグセットを取得
        /// @return タグセットへの参照
        [[nodiscard]] const TagSet& GetTags() const noexcept {
            return m_tags;
        }

        /// @brief タグ数を取得
        /// @return タグの数
        [[nodiscard]] std::size_t GetTagCount() const noexcept {
            return m_tags.size();
        }

    private:
        EntityId m_id;          ///< 一意識別子
        std::string m_name;     ///< 名前
        TagSet m_tags;          ///< タグセット
    };

    //============================================================
    // 定義済みタグ（任意）
    //============================================================

    namespace Tags {
        inline constexpr const char* kPlayer = "Player";
        inline constexpr const char* kEnemy = "Enemy";
        inline constexpr const char* kNpc = "NPC";
        inline constexpr const char* kItem = "Item";
        inline constexpr const char* kProjectile = "Projectile";
        inline constexpr const char* kTrigger = "Trigger";
        inline constexpr const char* kCamera = "Camera";
        inline constexpr const char* kLight = "Light";
        inline constexpr const char* kUI = "UI";
        inline constexpr const char* kStatic = "Static";
        inline constexpr const char* kDynamic = "Dynamic";
    } // namespace Tags

} // namespace Engine

// 使用例:
//
// // GameObjectでの使用例
// class GameObject {
// public:
//     EntityId GetId() const { return m_id; }
//     void SetName(const std::string& name) { m_name = name; }
//     const std::string& GetName() const { return m_name; }
//     void AddTag(const std::string& tag) { m_tags.insert(tag); }
//     bool HasTag(const std::string& tag) const { return m_tags.contains(tag); }
// private:
//     EntityId m_id = EntityId::Generate();
//     std::string m_name;
//     std::set<std::string> m_tags;
// };
//
// // 使用
// GameObject* player = scene->CreateObject<GameObject>();
// player->SetName("Player1");
// player->AddTag(Tags::kPlayer);
// player->AddTag(Tags::kDynamic);
//
// if (player->HasTag(Tags::kPlayer)) {
//     // プレイヤー固有の処理
// }
