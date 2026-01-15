#pragma once

#include <string_view>

namespace Engine {

    class Logger final {
    public:
        enum class LogLevel {
            Trace = 0,
            Info,
            Warn,
            Error,
            Off,
        };

        //============================================================
        // Lifecycle
        //============================================================
        static void Initialize();
        static void Finalize();

        //============================================================
        // Settings
        //============================================================
        static void SetLevel(LogLevel level);
        static LogLevel GetLevel();

        //============================================================
        // Logging
        //============================================================
        static void Trace(std::string_view message);
        static void Info(std::string_view message);
        static void Warn(std::string_view message);
        static void Error(std::string_view message);

    private:
        Logger() = delete;
        ~Logger() = delete;

        static void Write(LogLevel level, std::string_view message);
    };

} // namespace Engine
