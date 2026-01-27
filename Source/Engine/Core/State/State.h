#pragma once

/// @file   State.h
/// @brief  汎用ステートパターン実装

#include <memory>
#include <concepts>

namespace Engine {

    //============================================================
    // State Concept
    //============================================================

    /// @brief  ステートとして使用可能な型を制約
    /// @tparam T ステート型
    /// @tparam Context ステートが操作するコンテキスト型
    template<typename T, typename Context>
    concept StateConcept = requires(T t, Context& ctx, float dt) {
        { t.OnEnter(ctx) } -> std::same_as<void>;
        { t.OnExit(ctx) } -> std::same_as<void>;
        { t.Update(ctx, dt) } -> std::same_as<void>;
    };

    //============================================================
    // State Base
    //============================================================

    /// @brief  ステート基底クラス
    /// @tparam Context ステートが操作するコンテキスト型
    template<typename Context>
    class State {
    public:
        State() = default;
        virtual ~State() = default;

        State(const State&) = delete;
        State& operator=(const State&) = delete;

        /// @brief  ステート開始時に呼ばれる
        /// @param  context コンテキスト参照
        virtual void OnEnter(Context& context) {}

        /// @brief  ステート終了時に呼ばれる
        /// @param  context コンテキスト参照
        virtual void OnExit(Context& context) {}

        /// @brief  毎フレーム更新
        /// @param  context   コンテキスト参照
        /// @param  deltaTime 前フレームからの経過時間（秒）
        virtual void Update(Context& context, float deltaTime) {}

        /// @brief  毎フレーム描画
        /// @param  context コンテキスト参照
        virtual void Draw(Context& context) {}
    };

    //============================================================
    // State Machine
    //============================================================

    /// @brief  ステートマシン
    /// @tparam Context ステートが操作するコンテキスト型
    template<typename Context>
    class StateMachine final {
    public:
        /// @brief  コンストラクタ
        /// @param  context ステートが操作するコンテキスト参照
        explicit StateMachine(Context& context)
            : m_context(context) {}

        ~StateMachine() = default;

        StateMachine(const StateMachine&) = delete;
        StateMachine& operator=(const StateMachine&) = delete;

        //============================================================
        // State Control
        //============================================================

        /// @brief  ステートを変更（次フレームで適用）
        /// @param  newState 新しいステート
        void ChangeState(std::unique_ptr<State<Context>> newState) {
            m_nextState = std::move(newState);
        }

        /// @brief  ステートを変更（テンプレート版）
        /// @tparam T 新しいステート型
        /// @param  args ステートのコンストラクタ引数
        template<typename T, typename... Args>
        void ChangeStateTo(Args&&... args) {
            static_assert(std::is_base_of_v<State<Context>, T>, "T must derive from State<Context>");
            ChangeState(std::make_unique<T>(std::forward<Args>(args)...));
        }

        //============================================================
        // Frame Update
        //============================================================

        /// @brief  毎フレーム更新
        /// @param  deltaTime 前フレームからの経過時間（秒）
        void Update(float deltaTime) {
            ApplyPendingStateChange();

            if (m_currentState) {
                m_currentState->Update(m_context, deltaTime);
            }
        }

        /// @brief  毎フレーム描画
        void Draw() {
            if (m_currentState) {
                m_currentState->Draw(m_context);
            }
        }

        //============================================================
        // Getters
        //============================================================

        /// @brief  現在のステートを取得
        /// @return 現在のステートのポインタ（なければnullptr）
        [[nodiscard]] State<Context>* GetCurrentState() const noexcept {
            return m_currentState.get();
        }

        /// @brief  ステートが設定されているかを返す
        /// @return ステートが設定されていればtrue
        [[nodiscard]] bool HasState() const noexcept {
            return m_currentState != nullptr;
        }

        /// @brief  ステート変更が予約されているかを返す
        /// @return 予約されていればtrue
        [[nodiscard]] bool HasPendingState() const noexcept {
            return m_nextState != nullptr;
        }

    private:
        /// @brief  予約されたステート変更を適用
        void ApplyPendingStateChange() {
            if (!m_nextState) return;

            if (m_currentState) {
                m_currentState->OnExit(m_context);
            }

            m_currentState = std::move(m_nextState);
            m_currentState->OnEnter(m_context);
        }

    private:
        Context&                        m_context;          ///< コンテキスト参照
        std::unique_ptr<State<Context>> m_currentState;     ///< 現在のステート
        std::unique_ptr<State<Context>> m_nextState;        ///< 次のステート（予約）
    };

} // namespace Engine
