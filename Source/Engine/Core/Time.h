#pragma once

#include <cstdint>

namespace Engine {

    /**
     * @brief フレーム時間管理クラス
     *
     * - Tick() を毎フレーム呼ぶ
     * - deltaTime / totalTime / fps を取得できる
     */
    class Time final {
    public:
        Time() = default;
        ~Time() = default;

        Time(const Time&) = delete;
        Time& operator=(const Time&) = delete;

        //============================================================
        // Lifecycle
        //============================================================
        void Initialize();
        void Finalize();
        void Reset();

        //============================================================
        // Frame
        //============================================================
        void Tick();

        //============================================================
        // Getters (const)
        //============================================================
        float GetDeltaTime() const;     // 秒
        double GetTotalTime() const;    // 秒
        float GetFps() const;           // 直近1秒程度の概算FPS
        std::uint64_t GetFrameCount() const;

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
        static constexpr double kMaxDeltaTime = 0.25; // ブレーク等で巨大dtになるのを防ぐ(秒)
    };

} // namespace Engine
