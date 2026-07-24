#pragma once

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace cippie
{
    struct CacheEntry
    {
        std::string targetName;
        std::filesystem::path sourcePath;
        std::filesystem::path objectPath;
        std::filesystem::path depFilePath;
        std::string cacheKey;
        std::vector<std::pair<std::filesystem::path, std::string>> dependencies; // (path, hash)
    };
}
