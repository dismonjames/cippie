#pragma once

#include <cippie/core/Result.hpp>
#include <cippie/toolchain/Toolchain.hpp>

#include <filesystem>
#include <string>
#include <optional>

namespace cippie
{
    struct DetectOptions
    {
        std::optional<std::string> toolchainName;
        std::optional<std::string> targetTripleStr;
    };

    class ToolchainDetector
    {
    public:
        ToolchainDetector() = default;

        [[nodiscard]] Result<Toolchain> detect(const DetectOptions& options = {}) const;
    };
}
