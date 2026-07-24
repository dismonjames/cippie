#pragma once

#include <cippie/toolchain/Toolchain.hpp>

namespace cippie
{
    class ToolchainDetector
    {
    public:
        [[nodiscard]] Toolchain detect() const;
    };
}
