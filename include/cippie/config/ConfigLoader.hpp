#pragma once

#include <cippie/project/Project.hpp>

#include <filesystem>

namespace cippie
{
    class ConfigLoader
    {
    public:
        [[nodiscard]] Project load(
            const std::filesystem::path& projectRoot
        ) const;
    };
}
