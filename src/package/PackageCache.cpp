#include <cippie/package/PackageCache.hpp>
#include <cippie/cache/FileHasher.hpp>

#include <cstdlib>

namespace cippie
{
    std::filesystem::path PackageCache::getCacheDirectory()
    {
        const char* envCache = std::getenv("CIPPIE_CACHE_DIR");
        if (envCache != nullptr && envCache[0] != '\0')
        {
            return std::filesystem::path(envCache).lexically_normal();
        }

        const char* envHome = std::getenv("HOME");
        if (envHome != nullptr && envHome[0] != '\0')
        {
            return (std::filesystem::path(envHome) / ".cache/cippie").lexically_normal();
        }

        return std::filesystem::temp_directory_path() / "cippie-cache";
    }

    std::filesystem::path PackageCache::getDownloadsDirectory(const std::filesystem::path& baseCache)
    {
        return baseCache / "downloads";
    }

    std::filesystem::path PackageCache::getGitSourceDirectory(
        const std::string& url,
        const std::string& commit,
        const std::filesystem::path& baseCache
    )
    {
        std::string urlHash = FileHasher::hashString(url);
        return baseCache / "sources" / "git" / urlHash / commit;
    }

    std::filesystem::path PackageCache::getRegistrySourceDirectory(
        const std::string& name,
        const std::string& version,
        const std::filesystem::path& baseCache
    )
    {
        return baseCache / "sources" / "registry" / name / version;
    }

    std::filesystem::path PackageCache::getCompiledPackageDirectory(
        const std::string& targetTriple,
        const std::string& config,
        const std::string& name,
        const std::string& version,
        const std::filesystem::path& baseCache
    )
    {
        return baseCache / "packages" / targetTriple / config / name / version;
    }
}
