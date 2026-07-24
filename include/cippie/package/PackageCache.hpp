#pragma once

#include <filesystem>
#include <string>

namespace cippie
{
    class PackageCache
    {
    public:
        PackageCache() = default;

        [[nodiscard]] static std::filesystem::path getCacheDirectory();

        [[nodiscard]] static std::filesystem::path getDownloadsDirectory(
            const std::filesystem::path& baseCache = getCacheDirectory()
        );

        [[nodiscard]] static std::filesystem::path getGitSourceDirectory(
            const std::string& url,
            const std::string& commit,
            const std::filesystem::path& baseCache = getCacheDirectory()
        );

        [[nodiscard]] static std::filesystem::path getRegistrySourceDirectory(
            const std::string& name,
            const std::string& version,
            const std::filesystem::path& baseCache = getCacheDirectory()
        );

        [[nodiscard]] static std::filesystem::path getCompiledPackageDirectory(
            const std::string& targetTriple,
            const std::string& config,
            const std::string& name,
            const std::string& version,
            const std::filesystem::path& baseCache = getCacheDirectory()
        );
    };
}
