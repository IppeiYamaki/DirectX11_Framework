/// @file   GameObject.cpp
/// @brief  UnityライクなGameObject基底クラスの実装

#include "GameObject.h"

#include "Engine/Scene/Components/Transform.h"
#include "Engine/Scene/Component.h"

namespace Engine {

    GameObject::GameObject(const std::string& name)
        : m_name(name)
        , m_tag("")
        , m_isActive(true)
        , m_hasInitialized(false)
        , m_world(nullptr)
        , m_transform(nullptr) {
    }

    GameObject::~GameObject() {
        DestroyComponents();
    }

    //============================================================
    // Lifecycle
    //============================================================

    void GameObject::Initialize() {
        // 派生クラスでオーバーライドして初期化処理を記述
    }

    void GameObject::Update(float deltaTime) {
        (void)deltaTime;
        // 派生クラスでオーバーライドして更新処理を記述
    }

    void GameObject::LateUpdate(float deltaTime) {
        (void)deltaTime;
        // 派生クラスでオーバーライドして遅延更新処理を記述
    }

    void GameObject::Render() {
        // 派生クラスでオーバーライドして描画処理を記述
    }

    void GameObject::OnDestroy() {
        // 派生クラスでオーバーライドして破棄処理を記述
    }

    //============================================================
    // Transform
    //============================================================

    void GameObject::SetPosition(const Vector3& position) {
        if (m_transform) {
            m_transform->SetPosition(position);
        }
    }

    void GameObject::SetPosition(float x, float y, float z) {
        if (m_transform) {
            m_transform->SetPosition(x, y, z);
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

    void GameObject::SetUniformScale(float uniformScale) {
        if (m_transform) {
            m_transform->SetUniformScale(uniformScale);
        }
    }

    Vector3 GameObject::GetScale() const {
        if (m_transform) {
            return m_transform->GetScale();
        }
        return Vector3::One();
    }

    Transform* GameObject::GetTransform() {
        return m_transform;
    }

    const Transform* GameObject::GetTransform() const {
        return m_transform;
    }

    //============================================================
    // Identification
    //============================================================

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
    // Enable/Disable
    //============================================================

    void GameObject::SetActive(bool active) {
        m_isActive = active;
    }

    bool GameObject::IsActive() const {
        return m_isActive;
    }

    //============================================================
    // Component Management
    //============================================================

    std::size_t GameObject::GetComponentCount() const {
        return m_components.size();
    }

    //============================================================
    // World Access
    //============================================================

    void GameObject::SetWorld(World* world) {
        m_world = world;
    }

    World* GameObject::GetWorld() const {
        return m_world;
    }

    //============================================================
    // Internal
    //============================================================

    void GameObject::InternalInitialize() {
        if (m_hasInitialized) return;

        // Transformコンポーネントを追加
        auto transform = std::make_unique<Transform>();
        m_transform = transform.get();
        transform->SetOwner(this);
        m_components.emplace_back(std::move(transform));

        // 全コンポーネントのAwakeを呼ぶ
        for (auto& c : m_components) {
            c->OnAwake();
        }

        // 派生クラスの初期化処理を呼ぶ
        Initialize();

        // 全コンポーネントのStartを呼ぶ
        for (auto& c : m_components) {
            c->OnStart();
        }

        m_hasInitialized = true;
    }

    void GameObject::UpdateComponents(float deltaTime) {
        if (!m_isActive) return;

        for (auto& c : m_components) {
            if (c->IsEnabled()) {
                c->Update(deltaTime);
            }
        }
    }

    void GameObject::LateUpdateComponents(float deltaTime) {
        if (!m_isActive) return;

        for (auto& c : m_components) {
            if (c->IsEnabled()) {
                c->LateUpdate(deltaTime);
            }
        }
    }

    void GameObject::DrawComponents() {
        if (!m_isActive) return;

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
        m_transform = nullptr;
    }

} // namespace Engine
