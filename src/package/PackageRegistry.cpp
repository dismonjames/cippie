#include <cippie/package/PackageRegistry.hpp>

#include <cippie/config/ConfigLoader.hpp>
#include <cippie/package/HttpFetcher.hpp>
#include <cippie/package/PackageCache.hpp>
#include <cippie/package/PackageLock.hpp>
#include <cippie/util/ArchiveExtractor.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

namespace cippie
{
    PackageRegistry::PackageRegistry(std::string registryUrl)
        : m_registryUrl(std::move(registryUrl))
    {
        if (m_registryUrl.empty())
        {
            const char* envReg = std::getenv("CIPPIE_REGISTRY_URL");
            if (envReg != nullptr && envReg[0] != '\0')
            {
                m_registryUrl = envReg;
            }
        }
    }

    Result<std::vector<SemanticVersion>> PackageRegistry::listVersions(
        const std::string& packageName
    ) const
    {
        if (m_registryUrl.empty())
        {
            return std::unexpected(Error{
                .code = ErrorCode::validationFailed,
                .message = "no package registry URL specified",
                .location = std::nullopt,
                .notes = {}
            });
        }

        std::string indexUrl = m_registryUrl;
        if (!indexUrl.ends_with("/")) indexUrl += "/";
        indexUrl += "index/" + packageName + "/versions.txt";

        auto contentRes = HttpFetcher::fetchString(indexUrl);
        if (!contentRes.has_value())
        {
            return std::unexpected(contentRes.error());
        }

        std::vector<SemanticVersion> versions;
        std::stringstream ss(*contentRes);
        std::string line;
        while (std::getline(ss, line))
        {
            if (line.empty() || line.front() == '#') continue;
            auto vRes = SemanticVersion::parse(line);
            if (vRes.has_value())
            {
                versions.push_back(*vRes);
            }
        }

        std::sort(versions.begin(), versions.end());
        return versions;
    }

    Result<RegistryPackageMeta> PackageRegistry::fetchMetadata(
        const std::string& packageName,
        const SemanticVersion& version
    ) const
    {
        std::string metaUrl = m_registryUrl;
        if (!metaUrl.ends_with("/")) metaUrl += "/";
        metaUrl += "index/" + packageName + "/" + version.toString() + ".manifest";

        auto contentRes = HttpFetcher::fetchString(metaUrl);
        if (!contentRes.has_value())
        {
            return std::unexpected(contentRes.error());
        }

        RegistryPackageMeta meta;
        meta.name = packageName;
        meta.version = version;
        meta.manifestContent = *contentRes;

        std::stringstream ss(*contentRes);
        std::string line;
        while (std::getline(ss, line))
        {
            if (line.empty() || line.front() == '#') continue;
            std::stringstream lineSs(line);
            std::string key, val;
            lineSs >> key >> val;
            if (key == "sha256") meta.sha256 = val;
            else if (key == "url") meta.downloadUrl = val;
        }

        if (meta.downloadUrl.empty())
        {
            // Default construct relative package URL
            meta.downloadUrl = m_registryUrl;
            if (!meta.downloadUrl.ends_with("/")) meta.downloadUrl += "/";
            meta.downloadUrl += "packages/" + packageName + "/" + packageName + "-" + version.toString() + ".tar.gz";
        }

        return meta;
    }

    Result<std::filesystem::path> PackageRegistry::restorePackage(
        const std::string& packageName,
        const SemanticVersion& version,
        const std::filesystem::path& baseCache,
        bool offline
    ) const
    {
        const auto sourceDir = PackageCache::getRegistrySourceDirectory(packageName, version.toString(), baseCache);
        std::error_code ec;

        if (std::filesystem::is_regular_file(sourceDir / "Cippiefile", ec))
        {
            return sourceDir; // Warm cache hit
        }

        if (offline)
        {
            return std::unexpected(Error{
                .code = ErrorCode::validationFailed,
                .message = "offline package unavailable: package " + packageName + " " + version.toString() + " not found in local cache: " + sourceDir.string(),
                .location = std::nullopt,
                .notes = {}
            });
        }

        auto pkgLockRes = PackageLock::acquire(baseCache, packageName + "-" + version.toString());
        if (!pkgLockRes.has_value())
        {
            return std::unexpected(pkgLockRes.error());
        }

        // Re-check after lock acquired
        if (std::filesystem::is_regular_file(sourceDir / "Cippiefile", ec))
        {
            return sourceDir;
        }

        auto metaRes = fetchMetadata(packageName, version);
        if (!metaRes.has_value())
        {
            return std::unexpected(metaRes.error());
        }

        const auto downloadsDir = PackageCache::getDownloadsDirectory(baseCache);
        const auto archivePath = downloadsDir / (packageName + "-" + version.toString() + ".tar.gz");

        auto downloadRes = HttpFetcher::downloadFile(metaRes->downloadUrl, archivePath, metaRes->sha256);
        if (!downloadRes.has_value())
        {
            return std::unexpected(downloadRes.error());
        }

        auto extractRes = ArchiveExtractor::extract(archivePath, sourceDir);
        if (!extractRes.has_value())
        {
            return std::unexpected(extractRes.error());
        }

        return sourceDir;
    }
}
