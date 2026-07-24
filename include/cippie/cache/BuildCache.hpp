#pragma once

#include <filesystem>

namespace cippie
{
    class BuildCache
    {
    public:
        [[nodiscard]] bool isUpToDate(
            const std::filesystem::path& source,
            const std::filesystem::path& object
        ) const;
    };
}
