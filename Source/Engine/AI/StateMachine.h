/// @file   StateMachine.h
/// @brief  汎用ステートマシン
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

namespace Engine {

    //============================================================
    // ステートインターフェース
    //============================================================

    /// @brief  ステートインターフェース
    /// @tparam Context ステートで使用するコンテキスト型
    template <typename Context>
    class IState {
    public:
        virtual ~IState() = default;

        /// @brief ステート開始時に呼ばれる
        /// @param context コンテキスト参照
        virtual void OnEnter(Context& context) = 0;

        /// @brief ステート終了時に呼ばれる
        /// @param context コンテキスト参照
        virtual void OnExit(Context& context) = 0;

        /// @brief 毎フレーム更新
        /// @param context   コンテキスト参照
        /// @param deltaTime フレーム経過時間
        virtual void Update(Context& context, float deltaTime) = 0;
    };

    //============================================================
    // ステートマシン
    //============================================================

    /// @brief  汎用ステートマシン
    /// @tparam Context ステートで使用するコンテキスト型
    template <typename Context>
    class StateMachine final {
    public:
        using StatePtr = std::unique_ptr<IState<Context>>;
        using StateId = std::string;

        StateMachine() = default;
        ~StateMachine() = default;

        StateMachine(const StateMachine&) = delete;
        StateMachine& operator=(const StateMachine&) = delete;

        //============================================================
        // ステート登録
        //============================================================

        /// @brief ステートを登録
        /// @param id    ステート識別子
        /// @param state ステートオブジェクト
        void RegisterState(const StateId& id, StatePtr state) {
            m_states[id] = std::move(state);
        }

        /// @brief ステートをインプレイスで作成して登録
        /// @tparam StateType ステート型
        /// @tparam Args      コンストラクタ引数型
        /// @param  id        ステート識別子
        /// @param  args      コンストラクタ引数
        template <typename StateType, typename... Args>
        void RegisterState(const StateId& id, Args&&... args) {
            m_states[id] = std::make_unique<StateType>(std::forward<Args>(args)...);
        }

        /// @brief ステートを解除
        /// @param id ステート識別子
        void UnregisterState(const StateId& id) {
            // 現在のステートは解除しない
            if (m_currentStateId == id) return;
            m_states.erase(id);
        }

        //============================================================
        // ステート制御
        //============================================================

        /// @brief 初期ステートを設定して開始
        /// @param id      初期ステートの識別子
        /// @param context コンテキスト
        void Start(const StateId& id, Context& context) {
            auto it = m_states.find(id);
            if (it == m_states.end()) return;

            m_currentStateId = id;
            m_currentState = it->second.get();
            m_currentState->OnEnter(context);
            m_isRunning = true;
        }

        /// @brief ステートマシンを停止
        /// @param context コンテキスト
        void Stop(Context& context) {
            if (m_currentState && m_isRunning) {
                m_currentState->OnExit(context);
            }
            m_currentState = nullptr;
            m_currentStateId.clear();
            m_isRunning = false;
        }

        /// @brief 毎フレーム更新
        /// @param context   コンテキスト
        /// @param deltaTime フレーム経過時間
        void Update(Context& context, float deltaTime) {
            if (m_currentState && m_isRunning) {
                m_currentState->Update(context, deltaTime);
            }
        }

        /// @brief ステートを遷移
        /// @param id      遷移先ステートの識別子
        /// @param context コンテキスト
        void ChangeState(const StateId& id, Context& context) {
            auto it = m_states.find(id);
            if (it == m_states.end()) return;

            if (m_currentState) {
                m_currentState->OnExit(context);
            }

            m_currentStateId = id;
            m_currentState = it->second.get();
            m_currentState->OnEnter(context);
        }

        //============================================================
        // クエリ
        //============================================================

        /// @brief 現在のステートIDを取得
        /// @return 現在のステートID
        [[nodiscard]] const StateId& GetCurrentStateId() const {
            return m_currentStateId;
        }

        /// @brief 現在のステートを取得
        /// @return 現在のステートへのポインタ
        [[nodiscard]] IState<Context>* GetCurrentState() const {
            return m_currentState;
        }

        /// @brief ステートが登録されているか確認
        /// @param id ステート識別子
        /// @return 登録されていればtrue
        [[nodiscard]] bool HasState(const StateId& id) const {
            return m_states.contains(id);
        }

        /// @brief ステートマシンが実行中か確認
        /// @return 実行中ならtrue
        [[nodiscard]] bool IsRunning() const {
            return m_isRunning;
        }

        /// @brief 現在のステートか確認
        /// @param id ステート識別子
        /// @return 現在のステートならtrue
        [[nodiscard]] bool IsCurrentState(const StateId& id) const {
            return m_currentStateId == id;
        }

    private:
        std::unordered_map<StateId, StatePtr> m_states;
        IState<Context>* m_currentState = nullptr;
        StateId m_currentStateId;
        bool m_isRunning = false;
    };

    //============================================================
    // ラムダベースのステート
    //============================================================

    /// @brief  ラムダ式でステートを定義するためのヘルパークラス
    /// @tparam Context コンテキスト型
    template <typename Context>
    class LambdaState final : public IState<Context> {
    public:
        using EnterFunc = std::function<void(Context&)>;
        using ExitFunc = std::function<void(Context&)>;
        using UpdateFunc = std::function<void(Context&, float)>;

        /// @brief コンストラクタ
        /// @param onEnter  OnEnterコールバック
        /// @param onExit   OnExitコールバック
        /// @param onUpdate Updateコールバック
        LambdaState(EnterFunc onEnter = nullptr,
                    ExitFunc onExit = nullptr,
                    UpdateFunc onUpdate = nullptr)
            : m_onEnter(std::move(onEnter))
            , m_onExit(std::move(onExit))
            , m_onUpdate(std::move(onUpdate)) {}

        void OnEnter(Context& context) override {
            if (m_onEnter) m_onEnter(context);
        }

        void OnExit(Context& context) override {
            if (m_onExit) m_onExit(context);
        }

        void Update(Context& context, float deltaTime) override {
            if (m_onUpdate) m_onUpdate(context, deltaTime);
        }

    private:
        EnterFunc m_onEnter;
        ExitFunc m_onExit;
        UpdateFunc m_onUpdate;
    };

    //============================================================
    // ステートマシンビルダー
    //============================================================

    /// @brief  ステートマシンをビルダーパターンで構築
    /// @tparam Context コンテキスト型
    template <typename Context>
    class StateMachineBuilder final {
    public:
        using StateId = typename StateMachine<Context>::StateId;

        StateMachineBuilder() : m_stateMachine(std::make_unique<StateMachine<Context>>()) {}

        /// @brief ステートを追加
        /// @param id    ステート識別子
        /// @param state ステートオブジェクト
        /// @return ビルダー参照
        StateMachineBuilder& AddState(const StateId& id,
                                       std::unique_ptr<IState<Context>> state) {
            m_stateMachine->RegisterState(id, std::move(state));
            return *this;
        }

        /// @brief ラムダステートを追加
        /// @param id       ステート識別子
        /// @param onEnter  OnEnterコールバック
        /// @param onExit   OnExitコールバック
        /// @param onUpdate Updateコールバック
        /// @return ビルダー参照
        StateMachineBuilder& AddLambdaState(
            const StateId& id,
            typename LambdaState<Context>::EnterFunc onEnter = nullptr,
            typename LambdaState<Context>::ExitFunc onExit = nullptr,
            typename LambdaState<Context>::UpdateFunc onUpdate = nullptr) {
            m_stateMachine->RegisterState(
                id, std::make_unique<LambdaState<Context>>(
                    std::move(onEnter), std::move(onExit), std::move(onUpdate)));
            return *this;
        }

        /// @brief 初期ステートを設定
        /// @param id 初期ステートの識別子
        /// @return ビルダー参照
        StateMachineBuilder& SetInitialState(const StateId& id) {
            m_initialStateId = id;
            return *this;
        }

        /// @brief ステートマシンをビルド
        /// @return 構築されたステートマシン
        [[nodiscard]] std::unique_ptr<StateMachine<Context>> Build() {
            return std::move(m_stateMachine);
        }

        /// @brief ステートマシンをビルドして開始
        /// @param context コンテキスト
        /// @return 構築されたステートマシン
        [[nodiscard]] std::unique_ptr<StateMachine<Context>> BuildAndStart(Context& context) {
            if (!m_initialStateId.empty()) {
                m_stateMachine->Start(m_initialStateId, context);
            }
            return std::move(m_stateMachine);
        }

    private:
        std::unique_ptr<StateMachine<Context>> m_stateMachine;
        StateId m_initialStateId;
    };

} // namespace Engine

// 使用例:
//
// // コンテキスト構造体
// struct PlayerContext {
//     Transform* m_transform;
//     float m_speed;
//     bool m_isGrounded;
// };
//
// // ステートクラス
// class IdleState : public IState<PlayerContext> {
// public:
//     void OnEnter(PlayerContext& ctx) override { /* アニメーション設定など */ }
//     void OnExit(PlayerContext& ctx) override { /* クリーンアップ */ }
//     void Update(PlayerContext& ctx, float dt) override {
//         if (/* 移動入力あり */) {
//             // ステート遷移はStateMachine経由で行う
//         }
//     }
// };
//
// // ビルダーパターンでの構築
// auto sm = StateMachineBuilder<PlayerContext>()
//     .AddState("Idle", std::make_unique<IdleState>())
//     .AddState("Walk", std::make_unique<WalkState>())
//     .AddLambdaState("Jump",
//         [](auto& ctx) { /* OnEnter */ },
//         [](auto& ctx) { /* OnExit */ },
//         [](auto& ctx, float dt) { /* Update */ })
//     .SetInitialState("Idle")
//     .BuildAndStart(playerContext);
//
// // 更新ループ
// sm->Update(playerContext, deltaTime);
//
// // ステート遷移
// sm->ChangeState("Walk", playerContext);
