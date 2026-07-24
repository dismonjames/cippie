#pragma once

#include <string_view>

namespace cippie
{
    enum class OperatingSystem
    {
        linux,
        windows,
        macos,
        unknown
    };

    [[nodiscard]] constexpr OperatingSystem currentOperatingSystem() noexcept
    {
#if defined(__linux__)
        return OperatingSystem::linux;
#elif defined(_WIN32)
        return OperatingSystem::windows;
#elif defined(__APPLE__)
        return OperatingSystem::macos;
#else
        return OperatingSystem::unknown;
#endif
    }

    [[nodiscard]] constexpr std::string_view operatingSystemName(
        OperatingSystem system
    ) noexcept
    {
        switch (system)
        {
            case OperatingSystem::linux:
                return "linux";
            case OperatingSystem::windows:
                return "windows";
            case OperatingSystem::macos:
                return "macos";
            case OperatingSystem::unknown:
                return "unknown";
        }

        return "unknown";
    }
}
