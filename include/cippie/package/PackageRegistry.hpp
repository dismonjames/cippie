#pragma once

#include <cippie/package/SemanticVersion.hpp>
#include <cippie/project/Project.hpp>
#include <cippie/core/Result.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace cippie
{
    struct RegistryPackageMeta
    {
        std::string name;
        SemanticVersion version;
        std::string sha256;
        std::string downloadUrl;
        std::string manifestContent;
    };

    class PackageRegistry
    {
    public:
        explicit PackageRegistry(std::string registryUrl = "");

        [[nodiscard]] Result<std::vector<SemanticVersion>> listVersions(
            const std::string& packageName
        ) const;

        [[nodiscard]] Result<RegistryPackageMeta> fetchMetadata(
            const std::string& packageName,
            const SemanticVersion& version
        ) const;

        [[nodiscard]] Result<std::filesystem::path> restorePackage(
            const std::string& packageName,
            const SemanticVersion& version,
            const std::filesystem::path& baseCache,
            bool offline = false
        ) const;

        [[nodiscard]] const std::string& url() const noexcept { return m_registryUrl; }

    private:
        std::string m_registryUrl;
    };
}
