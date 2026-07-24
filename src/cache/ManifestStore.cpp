#include <cippie/cache/ManifestStore.hpp>

#include <fstream>
#include <iostream>
#include <sstream>

namespace cippie
{
    std::vector<CacheEntry> ManifestStore::load(
        const std::filesystem::path& manifestPath
    ) const
    {
        std::vector<CacheEntry> entries;
        std::ifstream file(manifestPath);

        if (!file.is_open())
        {
            return entries;
        }

        std::string line;
        CacheEntry currentEntry;
        size_t expectedDeps = 0;
        bool inEntry = false;

        while (std::getline(file, line))
        {
            if (line.empty() || line.front() == '#') continue;

            std::stringstream ss(line);
            std::string tag;
            ss >> tag;

            if (tag == "ENTRY")
            {
                if (inEntry)
                {
                    entries.push_back(std::move(currentEntry));
                    currentEntry = {};
                }

                std::string srcStr, objStr, depStr, keyStr;
                if (!(ss >> currentEntry.targetName >> srcStr >> objStr >> depStr >> keyStr >> expectedDeps))
                {
                    // Corrupted line: safe recovery by returning empty manifest
                    return {};
                }

                currentEntry.sourcePath = std::filesystem::path(srcStr).lexically_normal();
                currentEntry.objectPath = std::filesystem::path(objStr).lexically_normal();
                currentEntry.depFilePath = std::filesystem::path(depStr).lexically_normal();
                currentEntry.cacheKey = keyStr;
                inEntry = true;
            }
            else if (tag == "DEP")
            {
                if (!inEntry) return {};
                std::string depPathStr, depHashStr;
                if (!(ss >> depPathStr >> depHashStr))
                {
                    return {};
                }
                currentEntry.dependencies.emplace_back(
                    std::filesystem::path(depPathStr).lexically_normal(),
                    depHashStr
                );
            }
            else
            {
                // Unknown tag / malformed -> safe recovery
                return {};
            }
        }

        if (inEntry)
        {
            entries.push_back(std::move(currentEntry));
        }

        return entries;
    }

    Result<void> ManifestStore::save(
        const std::filesystem::path& manifestPath,
        const std::vector<CacheEntry>& entries
    ) const
    {
        std::error_code ec;
        std::filesystem::create_directories(manifestPath.parent_path(), ec);
        if (ec)
        {
            return std::unexpected(Error{
                .code = ErrorCode::fileReadFailed,
                .message = "failed to create manifest directory: " + ec.message(),
                .location = std::nullopt,
                .notes = {}
            });
        }

        const std::filesystem::path tmpPath = manifestPath.string() + ".tmp";
        std::ofstream file(tmpPath);
        if (!file.is_open())
        {
            return std::unexpected(Error{
                .code = ErrorCode::fileReadFailed,
                .message = "failed to create temporary manifest file: " + tmpPath.string(),
                .location = std::nullopt,
                .notes = {}
            });
        }

        for (const auto& entry : entries)
        {
            file << "ENTRY " << entry.targetName << " "
                 << entry.sourcePath.string() << " "
                 << entry.objectPath.string() << " "
                 << entry.depFilePath.string() << " "
                 << entry.cacheKey << " "
                 << entry.dependencies.size() << "\n";

            for (const auto& dep : entry.dependencies)
            {
                file << "DEP " << dep.first.string() << " " << dep.second << "\n";
            }
        }

        file.flush();
        file.close();

        if (file.fail())
        {
            return std::unexpected(Error{
                .code = ErrorCode::fileReadFailed,
                .message = "failed to write manifest content to: " + tmpPath.string(),
                .location = std::nullopt,
                .notes = {}
            });
        }

        std::filesystem::rename(tmpPath, manifestPath, ec);
        if (ec)
        {
            return std::unexpected(Error{
                .code = ErrorCode::fileReadFailed,
                .message = "failed to atomically rename manifest file: " + ec.message(),
                .location = std::nullopt,
                .notes = {}
            });
        }

        return {};
    }
}
