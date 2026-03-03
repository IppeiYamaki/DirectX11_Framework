/// @file   FadeSystem.h
/// @brief  シーン遷移・演出用のフェード機能を提供するシステム
#pragma once

namespace Engine {

    class RenderSystem;

    /// @brief フェードモード
    enum class FadeMode {
        None,       ///< フェードなし
        FadeIn,     ///< FadeIn（alpha: 1.0→0.0、画面が見えてくる）
        FadeOut,    ///< FadeOut（alpha: 0.0→1.0、画面が暗くなる）
    };

    /// @brief イージングタイプ
    enum class EasingType {
        Linear,         ///< 線形（等速）
        EaseIn,         ///< ゆっくり始まる（Quad）
        EaseOut,        ///< ゆっくり終わる（Quad）
        EaseInOut,      ///< ゆっくり始まりゆっくり終わる（Quad）
        EaseInSine,     ///< サイン曲線（開始がゆっくり）
        EaseOutSine,    ///< サイン曲線（終了がゆっくり）
        EaseInOutSine,  ///< サイン曲線（開始・終了がゆっくり）
        EaseInCubic,    ///< 3次曲線（開始がゆっくり）
        EaseOutCubic,   ///< 3次曲線（終了がゆっくり）
        EaseInOutCubic, ///< 3次曲線（開始・終了がゆっくり）
    };

    /// @brief フェード状態
    enum class FadeState {
        Idle,       ///< 待機中（フェード未実行）
        Running,    ///< 実行中
        Done,       ///< 完了
    };

    /// @brief シーン遷移・演出用のフェードシステム
    /// @note  画面全体を覆う半透明オーバーレイでフェードイン/アウトを実現
    class FadeSystem final {
    public:
        FadeSystem() = default;
        ~FadeSystem();

        FadeSystem(const FadeSystem&) = delete;
        FadeSystem& operator=(const FadeSystem&) = delete;

        //============================================================
        // Lifecycle
        //============================================================

        /// @brief  初期化
        /// @param  renderSystem 描画システムへの参照
        /// @param  screenWidth 画面幅
        /// @param  screenHeight 画面高さ
        /// @return 成功ならtrue
        [[nodiscard]] bool Initialize(RenderSystem* renderSystem, float screenWidth, float screenHeight);

        /// @brief 終了処理
        void Finalize();

        //============================================================
        // Fade Control API
        //============================================================

        /// @brief  フェードを開始
        /// @param  mode フェードモード（FadeIn/FadeOut）
        /// @param  easing イージングタイプ
        /// @param  duration フェード所要時間（秒）
        void SetFade(FadeMode mode, EasingType easing, float duration);

        /// @brief  現在のフェードアルファ値を取得
        /// @return アルファ値（0.0～1.0）
        /// @note   FadeIn: 1.0→0.0、FadeOut: 0.0→1.0
        [[nodiscard]] float GetFadeAlpha() const;

        /// @brief  フェードが完了したか確認
        /// @return 完了済みならtrue、未完了/実行中ならfalse
        [[nodiscard]] bool IsFadeDone() const;

        /// @brief  フェードが実行中か確認
        /// @return 実行中ならtrue
        [[nodiscard]] bool IsFadeRunning() const;

        /// @brief フェードをキャンセル（即座に停止）
        void CancelFade();

        /// @brief フェードをリセット（初期状態に戻す）
        void ResetFade();

        //============================================================
        // Frame
        //============================================================

        /// @brief 毎フレーム更新
        /// @param deltaTime フレーム経過時間（秒）
        void Update(float deltaTime);

        /// @brief フェードオーバーレイを描画
        /// @note  UI描画の最後（最前面）に呼び出すこと
        void Draw();

        //============================================================
        // Settings
        //============================================================

        /// @brief フェード色を設定（デフォルト: 黒）
        /// @param r 赤成分（0.0～1.0）
        /// @param g 緑成分（0.0～1.0）
        /// @param b 青成分（0.0～1.0）
        void SetFadeColor(float r, float g, float b);

        /// @brief 画面サイズを更新
        /// @param width 画面幅
        /// @param height 画面高さ
        void SetScreenSize(float width, float height);

        //============================================================
        // State
        //============================================================

        /// @brief  初期化済みか確認
        /// @return 初期化済みならtrue
        [[nodiscard]] bool IsInitialized() const;

        /// @brief  現在のフェードモードを取得
        /// @return フェードモード
        [[nodiscard]] FadeMode GetCurrentMode() const;

        /// @brief  現在のフェード状態を取得
        /// @return フェード状態
        [[nodiscard]] FadeState GetCurrentState() const;

        /// @brief  フェードの進行率を取得（0.0～1.0）
        /// @return 進行率
        [[nodiscard]] float GetProgress() const;

    private:
        /// @brief イージング関数を適用
        /// @param t 進行率（0.0～1.0）
        /// @return イージング適用後の値
        [[nodiscard]] float ApplyEasing(float t) const;

    private:
        RenderSystem* m_renderSystem = nullptr;     ///< 描画システム（借用）

        float m_screenWidth = 1280.0f;              ///< 画面幅
        float m_screenHeight = 720.0f;              ///< 画面高さ

        // フェード状態
        FadeMode m_mode = FadeMode::None;           ///< 現在のフェードモード
        FadeState m_state = FadeState::Idle;        ///< 現在のフェード状態
        EasingType m_easing = EasingType::Linear;   ///< 現在のイージングタイプ

        float m_duration = 0.0f;                    ///< フェード所要時間（秒）
        float m_elapsed = 0.0f;                     ///< 経過時間（秒）
        float m_alpha = 0.0f;                       ///< 現在のアルファ値

        // フェード色（デフォルト: 黒）
        float m_fadeColorR = 0.0f;
        float m_fadeColorG = 0.0f;
        float m_fadeColorB = 0.0f;

        bool m_isInitialized = false;               ///< 初期化済みフラグ
    };

} // namespace Engine
