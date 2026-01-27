/// @file   GameObject.h
/// @brief  ゲーム内のすべてのオブジェクトの基底クラス（Unityスタイル）
#pragma once

#include <memory>
#include <vector>
#include <string>
#include <set>
#include <type_traits>

#include "Engine/Math/Vector3.h"
#include "Engine/Scene/Component.h"
#include "Engine/Scene/EntityId.h"

namespace Engine {

    class Transform;
    class Scene;

    /// @brief ゲーム内のすべてのオブジェクトの基底クラス
    /// @note  Transformや基本情報を持ち、派生クラスで具体的な挙動を定義する
    class GameObject {
    public:
        /// @brief コンストラクタ
        /// @param name オブジェクト名（オプション）
        explicit GameObject(const std::string& name = "");
        
        /// @brief デストラクタ
        virtual ~GameObject();

        // コピー禁止
        GameObject(const GameObject&) = delete;
        GameObject& operator=(const GameObject&) = delete;

        //============================================================
        // ライフサイクルメソッド（派生クラスでオーバーライド可能）
        //============================================================

        /// @brief 初期化処理（オブジェクト生成後に呼び出される）
        virtual void Initialize();

        /// @brief 毎フレームの更新処理
        /// @param deltaTime 前フレームからの経過時間（秒）
        virtual void Update(float deltaTime);

        /// @brief 遅延更新処理（Update後に呼び出される）
        /// @param deltaTime 前フレームからの経過時間（秒）
        virtual void LateUpdate(float deltaTime);

        /// @brief 描画処理
        virtual void Render();

        /// @brief 破棄処理（オブジェクト破棄前に呼び出される）
        virtual void OnDestroy();

        //============================================================
        // Transform操作
        //============================================================

        /// @brief 位置を設定
        /// @param position 設定する位置
        void SetPosition(const Vector3& position);

        /// @brief 位置を取得
        /// @return 現在の位置
        [[nodiscard]] Vector3 GetPosition() const;

        /// @brief 回転を設定（オイラー角、度単位）
        /// @param rotation 設定する回転
        void SetRotation(const Vector3& rotation);

        /// @brief 回転を取得（オイラー角、度単位）
        /// @return 現在の回転
        [[nodiscard]] Vector3 GetRotation() const;

        /// @brief スケールを設定
        /// @param scale 設定するスケール
        void SetScale(const Vector3& scale);

        /// @brief スケールを取得
        /// @return 現在のスケール
        [[nodiscard]] Vector3 GetScale() const;

        /// @brief 均一スケールを設定
        /// @param uniformScale 設定する均一スケール値
        void SetUniformScale(float uniformScale);

        /// @brief Transformコンポーネントを取得
        /// @return Transformへのポインタ
        [[nodiscard]] Transform* GetTransform();

        /// @brief Transformコンポーネントを取得（const版）
        /// @return Transformへのconstポインタ
        [[nodiscard]] const Transform* GetTransform() const;

        //============================================================
        // 識別情報
        //============================================================

        /// @brief オブジェクトIDを取得
        /// @return オブジェクトの一意識別子
        [[nodiscard]] EntityId GetId() const;

        /// @brief 名前を設定
        /// @param name オブジェクト名
        void SetName(const std::string& name);

        /// @brief 名前を取得
        /// @return オブジェクト名への参照
        [[nodiscard]] const std::string& GetName() const;

        /// @brief タグを設定（単一タグ方式、後方互換用）
        /// @param tag タグ文字列
        void SetTag(const std::string& tag);

        /// @brief タグを取得（最初のタグを返す、後方互換用）
        /// @return タグへの参照
        [[nodiscard]] const std::string& GetTag() const;

        /// @brief タグが一致するか確認
        /// @param tag 比較するタグ
        /// @return 一致すればtrue
        [[nodiscard]] bool CompareTag(const std::string& tag) const;

        /// @brief タグを追加
        /// @param tag 追加するタグ
        void AddTag(const Tag& tag);

        /// @brief タグを削除
        /// @param tag 削除するタグ
        void RemoveTag(const Tag& tag);

        /// @brief  タグを持っているか確認
        /// @param  tag チェックするタグ
        /// @return タグを持っていればtrue
        [[nodiscard]] bool HasTag(const Tag& tag) const;

        /// @brief 全タグを削除
        void ClearTags();

        /// @brief タグセットを取得
        /// @return タグセットへの参照
        [[nodiscard]] const TagSet& GetTags() const;

        //============================================================
        // 有効/無効制御
        //============================================================

        /// @brief オブジェクトを有効化
        void SetActive(bool active);

        /// @brief オブジェクトが有効か確認
        /// @return 有効ならtrue
        [[nodiscard]] bool IsActive() const;

        //============================================================
        // コンポーネント管理
        //============================================================

        /// @brief コンポーネントを追加
        /// @tparam T Componentを継承した型
        /// @tparam Args コンストラクタ引数型
        /// @param args コンストラクタ引数
        /// @return 追加されたコンポーネントへのポインタ
        template <class T, class... Args>
        [[nodiscard]] T* AddComponent(Args&&... args) {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

            auto comp = std::make_unique<T>(std::forward<Args>(args)...);
            T* raw = comp.get();

            raw->SetOwner(this);
            m_components.emplace_back(std::move(comp));

            raw->OnAwake();

            if (m_hasStarted) {
                raw->OnStart();
            }

            return raw;
        }

        /// @brief 最初に見つかったT型コンポーネントを返す
        /// @tparam T 検索するComponent型
        /// @return コンポーネントへのポインタ（見つからなければnullptr）
        template <class T>
        [[nodiscard]] T* GetComponent() {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            for (auto& c : m_components) {
                if (auto* t = dynamic_cast<T*>(c.get())) {
                    return t;
                }
            }
            return nullptr;
        }

        /// @brief 最初に見つかったT型コンポーネントを返す（const版）
        /// @tparam T 検索するComponent型
        /// @return コンポーネントへのconstポインタ
        template <class T>
        [[nodiscard]] const T* GetComponent() const {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            for (const auto& c : m_components) {
                if (auto* t = dynamic_cast<const T*>(c.get())) {
                    return t;
                }
            }
            return nullptr;
        }

        /// @brief 全てのT型コンポーネントを取得
        /// @tparam T 検索するComponent型
        /// @return コンポーネントのベクター
        template <class T>
        [[nodiscard]] std::vector<T*> GetComponents() {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            std::vector<T*> result;
            for (auto& c : m_components) {
                if (auto* t = dynamic_cast<T*>(c.get())) {
                    result.push_back(t);
                }
            }
            return result;
        }

        /// @brief T型のコンポーネントを全て削除
        /// @tparam T 削除するComponent型
        /// @return 削除した数
        template <class T>
        int RemoveComponents() {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

            int removed = 0;
            for (auto it = m_components.begin(); it != m_components.end(); ) {
                if (dynamic_cast<T*>(it->get()) != nullptr) {
                    (*it)->OnDestroy();
                    it = m_components.erase(it);
                    ++removed;
                }
                else {
                    ++it;
                }
            }
            return removed;
        }

        /// @brief T型のコンポーネントを持っているか確認
        /// @tparam T 検索するComponent型
        /// @return 持っていればtrue
        template <class T>
        [[nodiscard]] bool HasComponent() const {
            return GetComponent<T>() != nullptr;
        }

        /// @brief コンポーネント数を取得
        /// @return コンポーネント数
        [[nodiscard]] std::size_t GetComponentCount() const;

        //============================================================
        // Scene連携（内部用）
        //============================================================

        /// @brief 所属Sceneを設定（Scene側から呼び出し）
        /// @param scene 所属Scene
        void SetScene(Scene* scene);

        /// @brief 所属Sceneを取得
        /// @return 所属Sceneへのポインタ
        [[nodiscard]] Scene* GetScene() const;

    protected:
        //============================================================
        // 内部処理（Sceneから呼び出し）
        //============================================================
        
        /// @brief 開始処理が必要なら実行
        void StartIfNeeded();

        /// @brief コンポーネントの更新処理
        /// @param deltaTime フレーム経過時間
        void UpdateComponents(float deltaTime);

        /// @brief コンポーネントの遅延更新処理
        /// @param deltaTime フレーム経過時間
        void LateUpdateComponents(float deltaTime);

        /// @brief コンポーネントの描画処理
        void DrawComponents();

        /// @brief コンポーネントの破棄処理
        void DestroyComponents();

    private:
        // friend宣言（Sceneが内部メソッドを呼び出せるように）
        friend class Scene;

    protected:
        // 識別情報
        EntityId m_id;                                          ///< 一意識別子
        std::string m_name;                                     ///< オブジェクト名
        TagSet m_tags;                                          ///< タグセット

        // 状態フラグ
        bool m_isActive = true;                                 ///< 有効フラグ
        bool m_hasStarted = false;                              ///< Start済みフラグ

        // コンポーネント管理
        std::vector<std::unique_ptr<Component>> m_components;   ///< 所有コンポーネント群

        // Transform（便利のため直接参照を保持）
        Transform* m_transform = nullptr;                       ///< Transformへの参照

        // 所属Scene
        Scene* m_scene = nullptr;                               ///< 所属Scene（借用）
    };

} // namespace Engine
