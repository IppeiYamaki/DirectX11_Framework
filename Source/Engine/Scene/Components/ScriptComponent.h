#pragma once

#include "Engine/Scene/Component.h"

namespace Engine {

    /**
     * @brief ゲームロジック用のComponent基底（MonoBehaviour相当）
     *
     * - 今は “意味付け” が主目的（Componentと機能は同じ）
     * - 将来：スクリプト専用の補助APIを足していける
     */
    class ScriptComponent : public Component {
    public:
        ScriptComponent() = default;
        ~ScriptComponent() override = default;

        ScriptComponent(const ScriptComponent&) = delete;
        ScriptComponent& operator=(const ScriptComponent&) = delete;
    };

} // namespace Engine
