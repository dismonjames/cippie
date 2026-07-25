#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace cippie
{
    class SourceScanner
    {
    public:
        [[nodiscard]] std::vector<std::filesystem::path> scan(
            const std::filesystem::path& rootDir,
            const std::vector<std::string>& patterns,
            const std::optional<std::filesystem::path>& entry = std::nullopt
        ) const;

        [[nodiscard]] std::vector<std::filesystem::path> scan(
            const std::filesystem::path& sourceDirectory
        ) const;
    };
}
