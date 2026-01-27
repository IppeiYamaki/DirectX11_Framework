#pragma once

namespace Engine {

    class GameObject;

    /// @brief  機能単位の基底Component
    /// @note   owner(GameObject)とenabledを保持し、ライフサイクルを提供する
    class Component {
    public:
        Component() = default;
        virtual ~Component() = default;

        Component(const Component&) = delete;
        Component& operator=(const Component&) = delete;

        //============================================================
        // Owner (GameObject)
        //============================================================
        /// @brief  所属GameObjectを取得
        /// @return 所属GameObjectへのポインタ
        GameObject* GetGameObject();
        /// @brief  所属GameObjectを取得（const版）
        /// @return 所属GameObjectへのconstポインタ
        const GameObject* GetGameObject() const;

        /// @brief  所属GameObjectを取得（後方互換エイリアス）
        /// @return 所属GameObjectへのポインタ
        /// @note   以前はEntityを返していたが、GameObjectに統一された
        GameObject* GetOwner();
        /// @brief  所属GameObjectを取得（const版、後方互換エイリアス）
        /// @return 所属GameObjectへのconstポインタ
        const GameObject* GetOwner() const;

        //============================================================
        // Enable
        //============================================================
        /// @brief Componentを有効化
        void Enable();
        /// @brief Componentを無効化
        void Disable();
        /// @brief  Componentが有効か確認
        /// @return 有効ならtrue
        bool IsEnabled() const;

        //============================================================
        // Lifecycle hooks (override in derived components)
        //============================================================
        /// @brief Awake時に呼ばれる
        virtual void OnAwake();
        /// @brief Start時に呼ばれる
        virtual void OnStart();
        /// @brief 毎フレーム更新
        /// @param deltaTime フレーム経過時間
        virtual void Update(float deltaTime);
        /// @brief 毎フレーム遅延更新
        /// @param deltaTime フレーム経過時間
        virtual void LateUpdate(float deltaTime);
        /// @brief 毎フレーム描画
        virtual void Draw();
        /// @brief 破棄時に呼ばれる
        virtual void OnDestroy();

    private:
        /// @brief  所属GameObjectを設定
        /// @param  owner 所属GameObject
        void SetOwner(GameObject* owner);

    private:
        /// @brief 所属GameObject（借用）
        GameObject* m_gameObject = nullptr;
        /// @brief 有効フラグ
        bool m_isEnabled = true;

        // GameObjectからownerを設定できるようにする
        friend class GameObject;
    };

} // namespace Engine
