#pragma once

#include <cippie/core/Result.hpp>
#include <cippie/project/Project.hpp>

#include <filesystem>

namespace cippie
{
    class ConfigLoader
    {
    public:
        [[nodiscard]] Result<Project> load(
            const std::filesystem::path& projectRoot
        ) const;

        [[nodiscard]] Result<Project> loadFromFile(
            const std::filesystem::path& cippiefilePath
        ) const;
    };
}
