#pragma once
#include <string_view>

namespace x39
{
    enum class severity
    {
        na,
        fatal,
        error,
        warning,
        info,
        verbose,
        trace
    };

    inline static std::string_view to_string(severity sev)
    {
        using namespace std::string_view_literals;
        switch (sev)
        {
        case x39::severity::fatal: return "fatal"sv;
        case x39::severity::error: return "error"sv;
        case x39::severity::warning: return "warning"sv;
        case x39::severity::info: return "info"sv;
        case x39::severity::verbose: return "verbose"sv;
        case x39::severity::trace: return "trace"sv;
        default:return "unknown"sv;
        }
    }
}