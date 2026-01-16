

#include "Logger.h"

#include <Windows.h>
#include <mutex>
#include <string>
#include <chrono>
#include <ctime>

namespace Engine {

    namespace {
        std::mutex s_mutex;
        Logger::LogLevel s_level = Logger::LogLevel::Info;

        const char* ToLevelString(Logger::LogLevel level) {
            switch (level) {
            case Logger::LogLevel::Trace: return "TRACE";
            case Logger::LogLevel::Info:  return "INFO ";
            case Logger::LogLevel::Warn:  return "WARN ";
            case Logger::LogLevel::Error: return "ERROR";
            case Logger::LogLevel::Off:   return "OFF  ";
            default:                      return "UNKWN";
            }
        }

        std::string MakeTimeStamp() {
            using namespace std::chrono;

            const auto now = system_clock::now();
            const auto tt = system_clock::to_time_t(now);

            std::tm tm{};
            localtime_s(&tm, &tt);

            char buf[32]{};
            
			// Format: YYYY-MM-DD HH:MM:SS
            std::snprintf(
                buf, sizeof(buf),
                "%04d-%02d-%02d %02d:%02d:%02d",
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                tm.tm_hour, tm.tm_min, tm.tm_sec
            );
            return std::string(buf);
        }

        bool IsEnabled(Logger::LogLevel msgLevel) {
            if (s_level == Logger::LogLevel::Off) return false;
            return static_cast<int>(msgLevel) >= static_cast<int>(s_level);
        }
    }

    void Logger::Initialize() {
#ifdef _DEBUG
        s_level = LogLevel::Trace;
#else
        s_level = LogLevel::Info;
#endif
    }

    void Logger::Finalize() {

    }

    void Logger::SetLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_level = level;
    }

    Logger::LogLevel Logger::GetLevel() {
        std::lock_guard<std::mutex> lock(s_mutex);
        return s_level;
    }

    void Logger::Trace(std::string_view message) { Write(LogLevel::Trace, message); }
    void Logger::Info(std::string_view message) { Write(LogLevel::Info, message); }
    void Logger::Warn(std::string_view message) { Write(LogLevel::Warn, message); }
    void Logger::Error(std::string_view message) { Write(LogLevel::Error, message); }

    void Logger::Write(LogLevel level, std::string_view message) {
        std::lock_guard<std::mutex> lock(s_mutex);

        if (!IsEnabled(level)) return;

        std::string out;
        out.reserve(64 + message.size());

        out += "[";
        out += MakeTimeStamp();
        out += "][";
        out += ToLevelString(level);
        out += "] ";
        out.append(message.data(), message.size());
        out += "\n";

        OutputDebugStringA(out.c_str());
    }

} // namespace Engine
