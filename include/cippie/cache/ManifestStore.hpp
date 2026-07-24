#pragma once

#include <cippie/cache/CacheEntry.hpp>
#include <cippie/core/Result.hpp>

#include <filesystem>
#include <vector>

namespace cippie
{
    class ManifestStore
    {
    public:
        ManifestStore() = default;

        [[nodiscard]] std::vector<CacheEntry> load(
            const std::filesystem::path& manifestPath
        ) const;

        [[nodiscard]] Result<void> save(
            const std::filesystem::path& manifestPath,
            const std::vector<CacheEntry>& entries
        ) const;
    };
}
