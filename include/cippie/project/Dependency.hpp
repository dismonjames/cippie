#pragma once

#include <filesystem>
#include <string>

namespace cippie
{
    enum class PackageSourceType
    {
        registry,
        path,
        git
    };

    struct Dependency
    {
        std::string name;
        PackageSourceType sourceType{PackageSourceType::registry};
        std::string versionRequirement;
        std::filesystem::path path;
        std::string url;
        std::string tag;
        std::string rev;
        std::string branch;
    };
}
