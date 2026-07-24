#pragma once

#include <filesystem>
#include <string>

namespace cippie
{
    struct Dependency
    {
        std::string name;
        std::string versionRequirement;
        std::string type{"package"};
        std::string url;
        std::string tag;
        std::filesystem::path path;
    };
}
