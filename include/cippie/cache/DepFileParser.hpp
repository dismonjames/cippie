#pragma once

#include <cippie/core/Result.hpp>

#include <filesystem>
#include <vector>

namespace cippie
{
    class DepFileParser
    {
    public:
        DepFileParser() = default;

        [[nodiscard]] Result<std::vector<std::filesystem::path>> parse(
            const std::filesystem::path& depFilePath
        ) const;

        [[nodiscard]] static std::vector<std::filesystem::path> parseString(
            const std::string& content
        );
    };
}
