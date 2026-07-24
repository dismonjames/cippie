#pragma once

#include <string_view>

namespace cippie
{
    enum class TargetType
    {
        executable,
        staticLibrary,
        sharedLibrary,
        test
    };

    [[nodiscard]] constexpr std::string_view toString(TargetType type) noexcept
    {
        switch (type)
        {
            case TargetType::executable:
                return "executable";
            case TargetType::staticLibrary:
                return "staticLibrary";
            case TargetType::sharedLibrary:
                return "sharedLibrary";
            case TargetType::test:
                return "test";
        }
        return "executable";
    }
}
