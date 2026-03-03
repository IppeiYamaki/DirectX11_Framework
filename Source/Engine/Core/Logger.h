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
		/// @brief ロガーを初期化する
        static void Initialize();
		/// @brief ロガーを終了する
        static void Finalize();

        //============================================================
        // Settings
        //============================================================
        /// @brief ログレベルを設定する
		/// @param level ログレベル
        static void SetLevel(LogLevel level);
        /// @brief 現在のログレベルを取得する
		/// @return 現在のログレベル
        static LogLevel GetLevel();

        //============================================================
        // Logging
        //============================================================
		/// @brief トレースログを出力する
        /// @param message ログメッセージ
        static void Trace(std::string_view message);
        /// @brief インフォログを出力する
        /// @param message ログメッセージ
        static void Info(std::string_view message);
		/// @brief ワーニングログを出力する
		/// @param message ログメッセージ
        static void Warn(std::string_view message);
		/// @brief エラーログを出力する
		/// @param message ログメッセージ
        static void Error(std::string_view message);

    private:
        Logger() = delete;
        ~Logger() = delete;

        static void Write(LogLevel level, std::string_view message);
    };

} // namespace Engine
