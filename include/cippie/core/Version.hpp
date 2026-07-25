#pragma once

#include <string_view>

namespace cippie
{
    [[nodiscard]] constexpr std::string_view version() noexcept
    {
        return "0.1.0";
    }
}
