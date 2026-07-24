#pragma once

#include <cippie/toolchain/Toolchain.hpp>

namespace cippie
{
    class ToolchainDetector
    {
    public:
        ToolchainDetector() = default;

        [[nodiscard]] Toolchain detect() const;
    };
}
