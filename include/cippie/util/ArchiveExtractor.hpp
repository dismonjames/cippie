#pragma once

#include <cippie/core/Result.hpp>

#include <filesystem>

namespace cippie
{
    class ArchiveExtractor
    {
    public:
        ArchiveExtractor() = default;

        [[nodiscard]] static Result<void> extract(
            const std::filesystem::path& archivePath,
            const std::filesystem::path& destinationDirectory
        );

        [[nodiscard]] static bool isPathSafe(
            const std::filesystem::path& entryPath,
            const std::filesystem::path& extractionRoot
        ) noexcept;
    };
}
