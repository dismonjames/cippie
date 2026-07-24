#pragma once

#include <string_view>

namespace cippie
{
    enum class CompilerFamily
    {
        gcc,
        clang,
        msvc,
        unknown
    };

    [[nodiscard]] constexpr std::string_view toString(CompilerFamily family) noexcept
    {
        switch (family)
        {
            case CompilerFamily::gcc:
                return "gcc";
            case CompilerFamily::clang:
                return "clang";
            case CompilerFamily::msvc:
                return "msvc";
            case CompilerFamily::unknown:
                return "unknown";
        }
        return "unknown";
    }
}
