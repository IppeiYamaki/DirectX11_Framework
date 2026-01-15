#pragma once

namespace Engine {

    class Entity;

    /**
     * @brief Component 基底クラス（機能単位の土台）
     *
     * - owner（所属 Entity）と enabled を持つ
     * - Unity風ライフサイクル関数を提供（デフォルトは空）
     * - 外部から owner を勝手に差し替えられない設計
     */
    class Component {
    public:
        Component() = default;
        virtual ~Component() = default;

        Component(const Component&) = delete;
        Component& operator=(const Component&) = delete;

        //============================================================
        // Owner
        //============================================================
              Entity* GetOwner();
        const Entity* GetOwner() const;

        //============================================================
        // Enable
        //============================================================
        void Enable();
        void Disable();
        bool IsEnabled() const;

        //============================================================
        // Lifecycle hooks (override in derived components)
        //============================================================
        virtual void OnAwake();
        virtual void OnStart();
        virtual void Update(float deltaTime);
        virtual void LateUpdate(float deltaTime);
        virtual void Draw();
        virtual void OnDestroy();

    private:
        // Entity が所有と紐付けを管理するため、SetOwner は外に出さない
        void SetOwner(Entity* owner);

    private:
        Entity* m_owner     = nullptr;  // 所有 Entity（借用）
		bool    m_isEnabled = true;     // 有効化フラグ

        // Entity だけが owner を設定できるようにする
        friend class Entity;
    };

} // namespace Engine
