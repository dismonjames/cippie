#include <cippie/package/DependencyResolver.hpp>

#include <cippie/config/ConfigLoader.hpp>
#include <cippie/package/GitFetcher.hpp>
#include <cippie/package/PackageCache.hpp>
#include <cippie/package/PackageLock.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <queue>
#include <set>

namespace cippie
{
    DependencyResolver::DependencyResolver(
        PackageRegistry registry,
        const std::filesystem::path& baseCache
    )
        : m_registry(std::move(registry))
        , m_baseCache(baseCache)
    {
    }

    Result<ResolvedPackageGraph> DependencyResolver::resolve(
        const Project& rootProject,
        const std::optional<LockFile>& existingLock,
        bool offline,
        bool lockedOnly
    ) const
    {
        ResolvedPackageGraph graph;
        ConfigLoader configLoader;

        // Map of package name -> list of version requirements imposed by dependents
        std::map<std::string, std::vector<std::pair<std::string, VersionRequirement>>> constraints;
        std::map<std::string, Dependency> rootDeps;

        for (const auto& dep : rootProject.dependencies)
        {
            rootDeps[dep.name] = dep;
            auto reqRes = VersionRequirement::parse(dep.versionRequirement);
            if (!reqRes.has_value()) return std::unexpected(reqRes.error());
            constraints[dep.name].push_back({"root", *reqRes});
        }

        if (lockedOnly)
        {
            if (!existingLock.has_value())
            {
                return std::unexpected(Error{
                    .code = ErrorCode::validationFailed,
                    .message = "cannot run with --locked: Cippie.lock does not exist",
                    .location = std::nullopt,
                    .notes = {}
                });
            }

            for (const auto& dep : rootProject.dependencies)
            {
                const auto* lockedPkg = existingLock->findPackage(dep.name);
                if (lockedPkg == nullptr)
                {
                    return std::unexpected(Error{
                        .code = ErrorCode::validationFailed,
                        .message = "lock file mismatch: requirement for '" + dep.name + "' is not present in Cippie.lock",
                        .location = std::nullopt,
                        .notes = {}
                    });
                }
            }
        }

        std::queue<std::string> pendingQueue;
        for (const auto& dep : rootProject.dependencies)
        {
            pendingQueue.push(dep.name);
        }

        std::set<std::string> resolvingPath;

        while (!pendingQueue.empty())
        {
            std::string pkgName = pendingQueue.front();
            pendingQueue.pop();

            if (graph.packages.contains(pkgName))
            {
                continue;
            }

            // Check for path package or git package definition from root
            Dependency decl;
            auto rootIt = rootDeps.find(pkgName);
            if (rootIt != rootDeps.end())
            {
                decl = rootIt->second;
            }
            else
            {
                decl.name = pkgName;
                decl.sourceType = PackageSourceType::registry;
                decl.versionRequirement = "*";
            }

            if (decl.sourceType == PackageSourceType::path)
            {
                std::error_code ec;
                auto absPath = std::filesystem::weakly_canonical(rootProject.rootDirectory / decl.path, ec);
                if (!std::filesystem::is_regular_file(absPath / "Cippiefile", ec))
                {
                    return std::unexpected(Error{
                        .code = ErrorCode::fileReadFailed,
                        .message = "pathPackage '" + pkgName + "' not found at: " + absPath.string(),
                        .location = std::nullopt,
                        .notes = {}
                    });
                }

                auto pkgProjRes = configLoader.load(absPath);
                if (!pkgProjRes.has_value()) return std::unexpected(pkgProjRes.error());

                SemanticVersion pathVer{.major = 0, .minor = 1, .patch = 0, .prerelease = {}, .buildMetadata = {}};
                ResolvedPackage rPkg{
                    .name = pkgName,
                    .version = pathVer,
                    .sourceType = PackageSourceType::path,
                    .sourceLocation = absPath.string(),
                    .commit = "",
                    .integrity = "",
                    .sourceDirectory = absPath,
                    .dependencies = {}
                };

                for (const auto& subDep : pkgProjRes->dependencies)
                {
                    rPkg.dependencies.push_back(subDep.name);
                    rootDeps[subDep.name] = subDep;

                    auto reqRes = VersionRequirement::parse(subDep.versionRequirement);
                    if (reqRes.has_value())
                    {
                        constraints[subDep.name].push_back({pkgName, *reqRes});
                    }
                    if (!graph.packages.contains(subDep.name))
                    {
                        pendingQueue.push(subDep.name);
                    }
                }

                graph.packages[pkgName] = std::move(rPkg);
            }
            else if (decl.sourceType == PackageSourceType::git)
            {
                std::string selector = decl.tag;
                if (selector.empty()) selector = decl.rev;
                if (selector.empty()) selector = decl.branch;

                auto commitRes = GitFetcher::resolveRevision(decl.url, selector);
                if (!commitRes.has_value())
                {
                    if (offline)
                    {
                        return std::unexpected(Error{
                            .code = ErrorCode::validationFailed,
                            .message = "offline package unavailable: git package '" + pkgName + "' (" + decl.url + ")",
                            .location = std::nullopt,
                            .notes = {}
                        });
                    }
                    return std::unexpected(commitRes.error());
                }

                auto gitSourceDir = PackageCache::getGitSourceDirectory(decl.url, *commitRes, m_baseCache);
                auto fetchRes = GitFetcher::fetchAndCheckout(decl.url, *commitRes, gitSourceDir);
                if (!fetchRes.has_value()) return std::unexpected(fetchRes.error());

                auto pkgProjRes = configLoader.load(gitSourceDir);
                if (!pkgProjRes.has_value()) return std::unexpected(pkgProjRes.error());

                SemanticVersion gitVer{.major = 1, .minor = 0, .patch = 0, .prerelease = {}, .buildMetadata = {}};
                ResolvedPackage rPkg{
                    .name = pkgName,
                    .version = gitVer,
                    .sourceType = PackageSourceType::git,
                    .sourceLocation = decl.url,
                    .commit = *commitRes,
                    .integrity = "",
                    .sourceDirectory = gitSourceDir,
                    .dependencies = {}
                };

                for (const auto& subDep : pkgProjRes->dependencies)
                {
                    rPkg.dependencies.push_back(subDep.name);
                    rootDeps[subDep.name] = subDep;

                    auto reqRes = VersionRequirement::parse(subDep.versionRequirement);
                    if (reqRes.has_value())
                    {
                        constraints[subDep.name].push_back({pkgName, *reqRes});
                    }
                    if (!graph.packages.contains(subDep.name))
                    {
                        pendingQueue.push(subDep.name);
                    }
                }

                graph.packages[pkgName] = std::move(rPkg);
            }
            else // registry
            {
                const auto& activeConstraints = constraints[pkgName];
                std::vector<SemanticVersion> candidateVersions;

                if (existingLock.has_value())
                {
                    const auto* lockedPkg = existingLock->findPackage(pkgName);
                    if (lockedPkg != nullptr)
                    {
                        bool matchesAll = true;
                        for (const auto& c : activeConstraints)
                        {
                            if (!c.second.matches(lockedPkg->version))
                            {
                                matchesAll = false;
                                break;
                            }
                        }
                        if (matchesAll)
                        {
                            candidateVersions.push_back(lockedPkg->version);
                        }
                    }
                }

                if (candidateVersions.empty() && !offline)
                {
                    auto listRes = m_registry.listVersions(pkgName);
                    if (listRes.has_value())
                    {
                        auto available = *listRes;
                        std::sort(available.rbegin(), available.rend()); // descending order (highest version first)

                        for (const auto& ver : available)
                        {
                            bool matchesAll = true;
                            for (const auto& c : activeConstraints)
                            {
                                if (!c.second.matches(ver))
                                {
                                    matchesAll = false;
                                    break;
                                }
                            }
                            if (matchesAll)
                            {
                                candidateVersions.push_back(ver);
                            }
                        }
                    }
                }

                if (candidateVersions.empty())
                {
                    std::string err = "unable to resolve package '" + pkgName + "'\n\n";
                    for (const auto& c : activeConstraints)
                    {
                        err += "  " + c.first + " requires " + pkgName + " " + c.second.toString() + "\n";
                    }
                    err += "\n  no available version satisfies all requirements";

                    return std::unexpected(Error{
                        .code = ErrorCode::packageResolutionFailed,
                        .message = err,
                        .location = std::nullopt,
                        .notes = {}
                    });
                }

                SemanticVersion chosenVersion = candidateVersions.front();
                auto restoreRes = m_registry.restorePackage(pkgName, chosenVersion, m_baseCache, offline);

                if (!restoreRes.has_value())
                {
                    return std::unexpected(restoreRes.error());
                }

                std::filesystem::path pkgDir = *restoreRes;
                auto pkgProjRes = configLoader.load(pkgDir);
                if (!pkgProjRes.has_value()) return std::unexpected(pkgProjRes.error());

                auto metaRes = m_registry.fetchMetadata(pkgName, chosenVersion);
                std::string integrity = metaRes.has_value() ? metaRes->sha256 : "";

                ResolvedPackage rPkg{
                    .name = pkgName,
                    .version = chosenVersion,
                    .sourceType = PackageSourceType::registry,
                    .sourceLocation = m_registry.url(),
                    .commit = "",
                    .integrity = integrity,
                    .sourceDirectory = pkgDir,
                    .dependencies = {}
                };

                for (const auto& subDep : pkgProjRes->dependencies)
                {
                    rPkg.dependencies.push_back(subDep.name);
                    rootDeps[subDep.name] = subDep;

                    auto reqRes = VersionRequirement::parse(subDep.versionRequirement);
                    if (reqRes.has_value())
                    {
                        constraints[subDep.name].push_back({pkgName, *reqRes});
                    }
                    if (!graph.packages.contains(subDep.name))
                    {
                        pendingQueue.push(subDep.name);
                    }
                }

                graph.packages[pkgName] = std::move(rPkg);
            }
        }

        return graph;
    }
}
