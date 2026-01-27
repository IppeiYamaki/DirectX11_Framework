/// @file   GameObject.h
/// @brief  UnityライクなGameObject基底クラス
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Engine/Math/Vector3.h"
#include "Engine/Scene/Component.h"

namespace Engine {

    class Transform;
    class World;

    /// @brief ゲームオブジェクトの基底クラス（UnityのGameObject相当）
    /// @note  Transform情報を持ち、Component管理とライフサイクルを担当
    ///        派生クラスでゲーム固有の振る舞いを定義する
    class GameObject {
    public:
        /// @brief コンストラクタ
        /// @param name オブジェクト名
        explicit GameObject(const std::string& name = "");

        /// @brief デストラクタ
        virtual ~GameObject();

        // コピー禁止
        GameObject(const GameObject&) = delete;
        GameObject& operator=(const GameObject&) = delete;

        //============================================================
        // Lifecycle (override in derived classes)
        //============================================================

        /// @brief 初期化処理
        virtual void Initialize();

        /// @brief 毎フレームの処理
        /// @param deltaTime フレーム経過時間（秒）
        virtual void Update(float deltaTime);

        /// @brief 遅延更新処理（Update後に呼ばれる）
        /// @param deltaTime フレーム経過時間（秒）
        virtual void LateUpdate(float deltaTime);

        /// @brief 描画処理
        virtual void Render();

        /// @brief 破棄時の処理
        virtual void OnDestroy();

        //============================================================
        // Transform
        //============================================================

        /// @brief 位置を設定
        /// @param position 位置
        void SetPosition(const Vector3& position);

        /// @brief 位置を設定（個別指定）
        /// @param x X座標
        /// @param y Y座標
        /// @param z Z座標
        void SetPosition(float x, float y, float z);

        /// @brief 位置を取得
        /// @return 位置
        [[nodiscard]] Vector3 GetPosition() const;

        /// @brief 回転を設定（オイラー角・度単位）
        /// @param rotation 回転（度単位）
        void SetRotation(const Vector3& rotation);

        /// @brief 回転を取得（オイラー角・度単位）
        /// @return 回転（度単位）
        [[nodiscard]] Vector3 GetRotation() const;

        /// @brief スケールを設定
        /// @param scale スケール
        void SetScale(const Vector3& scale);

        /// @brief 均一スケールを設定
        /// @param uniformScale 均一スケール値
        void SetUniformScale(float uniformScale);

        /// @brief スケールを取得
        /// @return スケール
        [[nodiscard]] Vector3 GetScale() const;

        /// @brief Transformコンポーネントを取得
        /// @return Transformへのポインタ
        [[nodiscard]] Transform* GetTransform();

        /// @brief Transformコンポーネントを取得（const版）
        /// @return Transformへのconstポインタ
        [[nodiscard]] const Transform* GetTransform() const;

        //============================================================
        // Identification
        //============================================================

        /// @brief 名前を設定
        /// @param name オブジェクト名
        void SetName(const std::string& name);

        /// @brief 名前を取得
        /// @return オブジェクト名
        [[nodiscard]] const std::string& GetName() const;

        /// @brief タグを設定
        /// @param tag タグ
        void SetTag(const std::string& tag);

        /// @brief タグを取得
        /// @return タグ
        [[nodiscard]] const std::string& GetTag() const;

        /// @brief タグと比較
        /// @param tag 比較するタグ
        /// @return 一致すればtrue
        [[nodiscard]] bool CompareTag(const std::string& tag) const;

        //============================================================
        // Enable/Disable
        //============================================================

        /// @brief オブジェクトを有効化
        void SetActive(bool active);

        /// @brief オブジェクトが有効か確認
        /// @return 有効ならtrue
        [[nodiscard]] bool IsActive() const;

        //============================================================
        // Component Management
        //============================================================

        /// @brief Componentを追加
        /// @tparam T Componentを継承した型
        /// @tparam Args コンストラクタ引数型
        /// @param args コンストラクタ引数
        /// @return 追加されたComponentへのポインタ
        template <class T, class... Args>
        [[nodiscard]] T* AddComponent(Args&&... args) {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

            auto comp = std::make_unique<T>(std::forward<Args>(args)...);
            T* raw = comp.get();

            raw->SetOwner(this);
            m_components.emplace_back(std::move(comp));

            if (m_hasInitialized) {
                raw->OnAwake();
                raw->OnStart();
            }

            return raw;
        }

        /// @brief 最初に見つかったT型Componentを返す
        /// @tparam T 検索するComponent型
        /// @return Componentへのポインタ（見つからなければnullptr）
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

        /// @brief 最初に見つかったT型Componentを返す（const版）
        /// @tparam T 検索するComponent型
        /// @return Componentへのconstポインタ
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

        /// @brief 全てのT型Componentを取得
        /// @tparam T 検索するComponent型
        /// @return Componentのベクター
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

        /// @brief T型のComponentを持っているか確認
        /// @tparam T 検索するComponent型
        /// @return 持っていればtrue
        template <class T>
        [[nodiscard]] bool HasComponent() const {
            return GetComponent<T>() != nullptr;
        }

        /// @brief T型のComponentを削除
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

        /// @brief Component数を取得
        /// @return Component数
        [[nodiscard]] std::size_t GetComponentCount() const;

        //============================================================
        // World Access
        //============================================================

        /// @brief 所属Worldを設定
        /// @param world 所属World
        void SetWorld(World* world);

        /// @brief 所属Worldを取得
        /// @return 所属World
        [[nodiscard]] World* GetWorld() const;

    protected:
        /// @brief 内部初期化処理（World::CreateGameObjectから呼ばれる）
        void InternalInitialize();

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

    protected:
        std::string m_name;                                     ///< オブジェクト名
        std::string m_tag;                                      ///< タグ
        bool m_isActive = true;                                 ///< 有効フラグ
        bool m_hasInitialized = false;                          ///< 初期化済みフラグ

        World* m_world = nullptr;                               ///< 所属World（借用）
        Transform* m_transform = nullptr;                       ///< Transformへのキャッシュ

        std::vector<std::unique_ptr<Component>> m_components;   ///< 所有Component群

        // Worldからの内部アクセスを許可
        friend class World;
    };

} // namespace Engine
