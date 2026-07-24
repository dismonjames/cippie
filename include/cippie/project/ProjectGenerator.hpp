#pragma once

#include <cippie/core/Result.hpp>

#include <filesystem>
#include <string>
#include <string_view>

namespace cippie
{
    class ProjectGenerator
    {
    public:
        ProjectGenerator() = default;

        [[nodiscard]] static bool isValidProjectName(std::string_view name) noexcept;

        [[nodiscard]] Result<std::filesystem::path> generate(
            std::string_view projectName,
            const std::filesystem::path& parentDirectory
        ) const;
    };
}
