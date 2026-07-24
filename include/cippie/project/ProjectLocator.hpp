#pragma once

#include <filesystem>
#include <optional>

namespace cippie
{
    class ProjectLocator
    {
    public:
        [[nodiscard]] std::optional<std::filesystem::path> locate(
            const std::filesystem::path& startDirectory
        ) const;
    };
}
