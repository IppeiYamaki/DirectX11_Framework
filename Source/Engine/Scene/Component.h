#pragma once

namespace Engine {

    class Entity;

    /// @brief  機能単位の基底Component
    /// @note   owner(Entity)とenabledを保持し、ライフサイクルを提供する
    class Component {
    public:
        Component() = default;
        virtual ~Component() = default;

        Component(const Component&) = delete;
        Component& operator=(const Component&) = delete;

        //============================================================
        // Owner
        //============================================================
        /// @brief  所属Entityを取得
        /// @return 所属Entityへのポインタ
        Entity* GetOwner();
        /// @brief  所属Entityを取得（const版）
        /// @return 所属Entityへのconstポインタ
        const Entity* GetOwner() const;

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
        /// @brief  所属Entityを設定
        /// @param  owner 所属Entity
        void SetOwner(Entity* owner);

    private:
        /// @brief 所属Entity（借用）
        Entity* m_owner = nullptr;
        /// @brief 有効フラグ
        bool m_isEnabled = true;

        // Entity ������ owner ��ݒ�ł���悤�ɂ���
        friend class Entity;
    };

} // namespace Engine
