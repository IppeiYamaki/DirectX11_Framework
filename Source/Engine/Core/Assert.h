#pragma once

#include <Windows.h>     // HRESULT, FAILED, SUCCEEDED
#include <cstdio>        // std::snprintf
#include <string>
#include <string_view>

#include "Engine/Core/Logger.h"

namespace Engine::Assert {

    inline void DebugBreakNow() {
#if defined(_MSC_VER)
        __debugbreak();
#else
        DebugBreak();
#endif
    }

    inline void ReportFailure(
        std::string_view kind,
        const char* expression,
        const char* message,
        const char* file,
        int line,
        const char* function
    ) {
        char header[512]{};
        std::snprintf(
            header, sizeof(header),
            "[%.*s] %s(%d): %s\n  expr: %s\n",
            static_cast<int>(kind.size()), kind.data(),
            file, line, function, expression
        );

        std::string out = header;

        if (message != nullptr && message[0] != '\0') {
            out += "  msg : ";
            out += message;
            out += "\n";
        }

        Engine::Logger::Error(out);
    }

    inline void ReportHrFailure(
        HRESULT hr,
        const char* expression,
        const char* file,
        int line,
        const char* function
    ) {
        char buf[512]{};
        std::snprintf(
            buf, sizeof(buf),
            "[HR] %s(%d): %s\n  expr: %s\n  hr  : 0x%08lX\n",
            file, line, function, expression, static_cast<unsigned long>(hr)
        );
        Engine::Logger::Error(buf);
    }

} // namespace Engine::Assert

//============================================================
// Macros
//============================================================

// ASSERT：デバッグ時のみ有効（失敗でログ＋ブレーク）
#if defined(_DEBUG)
#define ASSERT(condition) \
        do { \
            if (!(condition)) { \
                ::Engine::Assert::ReportFailure("ASSERT", #condition, nullptr, __FILE__, __LINE__, __func__); \
                ::Engine::Assert::DebugBreakNow(); \
            } \
        } while (0)

#define ASSERT_MSG(condition, message) \
        do { \
            if (!(condition)) { \
                ::Engine::Assert::ReportFailure("ASSERT", #condition, (message), __FILE__, __LINE__, __func__); \
                ::Engine::Assert::DebugBreakNow(); \
            } \
        } while (0)
#else
#define ASSERT(condition)        ((void)0)
#define ASSERT_MSG(condition, message) ((void)0)
#endif

// VERIFY：リリースでも式は評価（副作用は残す）。デバッグでは失敗でブレーク。
#if defined(_DEBUG)
#define VERIFY(condition) ASSERT(condition)
#else
#define VERIFY(condition) ((void)(condition))
#endif

// VERIFY_HR：HRESULT をチェック（デバッグは失敗でログ＋ブレーク、リリースは評価のみ）
#if defined(_DEBUG)
#define VERIFY_HR(hrExpr) \
        do { \
            const HRESULT hr__ = (hrExpr); \
            if (FAILED(hr__)) { \
                ::Engine::Assert::ReportHrFailure(hr__, #hrExpr, __FILE__, __LINE__, __func__); \
                ::Engine::Assert::DebugBreakNow(); \
            } \
        } while (0)
#else
#define VERIFY_HR(hrExpr) ((void)(hrExpr))
#endif
