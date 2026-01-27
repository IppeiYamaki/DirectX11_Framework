/// @file   GameObject.cpp
/// @brief  GameObject基底クラスの実装
#include "GameObject.h"

#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Scene.h"

namespace Engine {

    //============================================================
    // コンストラクタ/デストラクタ
    //============================================================

    GameObject::GameObject(const std::string& name)
        : m_id(EntityId::Generate())
        , m_name(name)
        , m_tag("")
        , m_isActive(true)
        , m_hasStarted(false)
        , m_transform(nullptr)
        , m_scene(nullptr) {
    }

    GameObject::~GameObject() {
        DestroyComponents();
    }

    //============================================================
    // ライフサイクルメソッド
    //============================================================

    void GameObject::Initialize() {
        // Transformコンポーネントを自動追加
        if (!m_transform) {
            m_transform = AddComponent<Transform>();
        }
    }

    void GameObject::Update(float deltaTime) {
        if (!m_isActive) return;

        StartIfNeeded();
        UpdateComponents(deltaTime);
    }

    void GameObject::LateUpdate(float deltaTime) {
        if (!m_isActive) return;

        LateUpdateComponents(deltaTime);
    }

    void GameObject::Render() {
        if (!m_isActive) return;

        DrawComponents();
    }

    void GameObject::OnDestroy() {
        DestroyComponents();
    }

    //============================================================
    // Transform操作
    //============================================================

    void GameObject::SetPosition(const Vector3& position) {
        if (m_transform) {
            m_transform->SetPosition(position);
        }
    }

    Vector3 GameObject::GetPosition() const {
        if (m_transform) {
            return m_transform->GetPosition();
        }
        return Vector3::Zero();
    }

    void GameObject::SetRotation(const Vector3& rotation) {
        if (m_transform) {
            m_transform->SetRotationEulerDegrees(rotation);
        }
    }

    Vector3 GameObject::GetRotation() const {
        if (m_transform) {
            return m_transform->GetRotationEulerDegrees();
        }
        return Vector3::Zero();
    }

    void GameObject::SetScale(const Vector3& scale) {
        if (m_transform) {
            m_transform->SetScale(scale);
        }
    }

    Vector3 GameObject::GetScale() const {
        if (m_transform) {
            return m_transform->GetScale();
        }
        return Vector3::One();
    }

    void GameObject::SetUniformScale(float uniformScale) {
        if (m_transform) {
            m_transform->SetUniformScale(uniformScale);
        }
    }

    Transform* GameObject::GetTransform() {
        return m_transform;
    }

    const Transform* GameObject::GetTransform() const {
        return m_transform;
    }

    //============================================================
    // 識別情報
    //============================================================

    EntityId GameObject::GetId() const {
        return m_id;
    }

    void GameObject::SetName(const std::string& name) {
        m_name = name;
    }

    const std::string& GameObject::GetName() const {
        return m_name;
    }

    void GameObject::SetTag(const std::string& tag) {
        m_tag = tag;
    }

    const std::string& GameObject::GetTag() const {
        return m_tag;
    }

    bool GameObject::CompareTag(const std::string& tag) const {
        return m_tag == tag;
    }

    //============================================================
    // 有効/無効制御
    //============================================================

    void GameObject::SetActive(bool active) {
        m_isActive = active;
    }

    bool GameObject::IsActive() const {
        return m_isActive;
    }

    //============================================================
    // コンポーネント管理
    //============================================================

    std::size_t GameObject::GetComponentCount() const {
        return m_components.size();
    }

    //============================================================
    // Scene連携
    //============================================================

    void GameObject::SetScene(Scene* scene) {
        m_scene = scene;
    }

    Scene* GameObject::GetScene() const {
        return m_scene;
    }

    //============================================================
    // 内部処理
    //============================================================

    void GameObject::StartIfNeeded() {
        if (m_hasStarted) return;

        for (auto& c : m_components) {
            c->OnStart();
        }
        m_hasStarted = true;
    }

    void GameObject::UpdateComponents(float deltaTime) {
        for (auto& c : m_components) {
            if (c->IsEnabled()) {
                c->Update(deltaTime);
            }
        }
    }

    void GameObject::LateUpdateComponents(float deltaTime) {
        for (auto& c : m_components) {
            if (c->IsEnabled()) {
                c->LateUpdate(deltaTime);
            }
        }
    }

    void GameObject::DrawComponents() {
        for (auto& c : m_components) {
            if (c->IsEnabled()) {
                c->Draw();
            }
        }
    }

    void GameObject::DestroyComponents() {
        for (auto& c : m_components) {
            c->OnDestroy();
        }
        m_components.clear();
    }

} // namespace Engine
