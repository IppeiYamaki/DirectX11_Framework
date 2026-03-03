#pragma once

#include <cstdint>

namespace Engine {

	/// @brief フレーム時間管理クラス
	/// @brief Tick() を毎フレーム呼ぶことで、deltaTime / totalTime / fps を取得できる
    class Time final {
    public:
        Time() = default;
        ~Time() = default;

        Time(const Time&) = delete;
        Time& operator=(const Time&) = delete;

        //============================================================
        // Lifecycle
        //============================================================
		/// @brief Time を初期化する
        void Initialize();
		/// @brief Time を終了する
        void Finalize();
        /// @brief Time をリセットする
        void Reset();

        //============================================================
        // Frame
        //============================================================
		/// @brief 毎フレーム呼ぶ。これを呼ぶと deltaTime / totalTime / fps が更新される
        void Tick();

        //============================================================
        // Getters (const)
        //============================================================
		/// @brief 前フレームからの経過時間を取得
        float GetDeltaTime() const;     // 秒
		/// @brief アプリケーション開始からの経過時間を取得
        double GetTotalTime() const;    // 秒
		/// @brief 直近1秒程度の概算fpsを取得
		/// @return fps
        float GetFps() const;
		/// @brief フレームカウントを取得
		/// @return フレームカウント
        std::uint64_t GetFrameCount() const;

		/// @brief Time が初期化されているか確認する
        bool IsInitialized() const;

    private:
		bool            m_isInitialized = false;    // 初期化済みフラグ

		// QPC 関連
        long long       m_frequency     = 0;        // QPC 周波数
        long long       m_prevCounter   = 0;        // 前フレームの QPC カウンタ

		// 時間情報
        float           m_deltaTime     = 0.0f;     // 秒 
        double          m_totalTime     = 0.0;      // 秒
        std::uint64_t   m_frameCount    = 0;        // フレームカウント

		// FPS 計算用
        double          m_fpsElapsed    = 0.0;      // FPS 計測用経過時間（秒）
        std::uint32_t   m_fpsFrames     = 0;        // FPS 計測用フレーム数
        float           m_fps           = 0.0f;     // FPS

    private:
        static constexpr double kMaxDeltaTime = 0.25; // break等で巨大dtになるのを防ぐ(秒)
    };

} // namespace Engine
