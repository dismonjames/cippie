#pragma once

#include <cippie/package/LockFile.hpp>
#include <cippie/package/PackageCache.hpp>
#include <cippie/package/PackageRegistry.hpp>
#include <cippie/package/SemanticVersion.hpp>
#include <cippie/package/VersionRequirement.hpp>
#include <cippie/project/Project.hpp>
#include <cippie/core/Result.hpp>

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace cippie
{
    struct ResolvedPackage
    {
        std::string name;
        SemanticVersion version;
        PackageSourceType sourceType{PackageSourceType::registry};
        std::string sourceLocation;
        std::string commit;
        std::string integrity;
        std::filesystem::path sourceDirectory;
        std::vector<std::string> dependencies; // names of resolved packages
    };

    struct ResolvedPackageGraph
    {
        std::map<std::string, ResolvedPackage> packages;
    };

    class DependencyResolver
    {
    public:
        explicit DependencyResolver(
            PackageRegistry registry = PackageRegistry(),
            const std::filesystem::path& baseCache = PackageCache::getCacheDirectory()
        );

        [[nodiscard]] Result<ResolvedPackageGraph> resolve(
            const Project& rootProject,
            const std::optional<LockFile>& existingLock = std::nullopt,
            bool offline = false,
            bool lockedOnly = false
        ) const;

    private:
        PackageRegistry m_registry;
        std::filesystem::path m_baseCache;
    };
}
