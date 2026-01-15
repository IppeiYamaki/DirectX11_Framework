#include "Time.h"

#include <Windows.h> // QueryPerformanceCounter / QueryPerformanceFrequency

namespace Engine {

    namespace {
        long long GetQpcNow() {
            LARGE_INTEGER counter{};
            ::QueryPerformanceCounter(&counter);
            return counter.QuadPart;
        }
    }

    void Time::Initialize() {
        if (m_isInitialized) return;

        LARGE_INTEGER freq{};
        ::QueryPerformanceFrequency(&freq);
        m_frequency = freq.QuadPart;

        m_prevCounter = GetQpcNow();

        m_deltaTime = 0.0f;
        m_totalTime = 0.0;
        m_frameCount = 0;

        m_fpsElapsed = 0.0;
        m_fpsFrames = 0;
        m_fps = 0.0f;

        m_isInitialized = true;
    }

    void Time::Finalize() {
        if (!m_isInitialized) return;

        m_isInitialized = false;
    }

    void Time::Reset() {
        if (!m_isInitialized) return;

        m_prevCounter = GetQpcNow();

        m_deltaTime = 0.0f;
        m_totalTime = 0.0;
        m_frameCount = 0;

        m_fpsElapsed = 0.0;
        m_fpsFrames = 0;
        m_fps = 0.0f;
    }

    void Time::Tick() {
        if (!m_isInitialized) return;

        const long long now = GetQpcNow();
        const long long diff = now - m_prevCounter;
        m_prevCounter = now;

        double dt = 0.0;
        if (m_frequency > 0) {
            dt = static_cast<double>(diff) / static_cast<double>(m_frequency);
        }

        // ブレーク/ウィンドウドラッグ等で巨大dtになるのを抑制
        if (dt < 0.0) dt = 0.0;
        if (dt > kMaxDeltaTime) dt = kMaxDeltaTime;

        m_deltaTime = static_cast<float>(dt);
        m_totalTime += dt;
        ++m_frameCount;

        // FPS（およそ1秒区間で更新）
        m_fpsElapsed += dt;
        ++m_fpsFrames;

        if (m_fpsElapsed >= 1.0) {
            m_fps = static_cast<float>(static_cast<double>(m_fpsFrames) / m_fpsElapsed);
            m_fpsElapsed = 0.0;
            m_fpsFrames = 0;
        }
    }

    float Time::GetDeltaTime() const {
        return m_deltaTime;
    }

    double Time::GetTotalTime() const {
        return m_totalTime;
    }

    float Time::GetFps() const {
        return m_fps;
    }

    std::uint64_t Time::GetFrameCount() const {
        return m_frameCount;
    }

    bool Time::IsInitialized() const {
        return m_isInitialized;
    }

} // namespace Engine
