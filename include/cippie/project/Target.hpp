#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace cippie
{
    enum class TargetType
    {
        executable,
        staticLibrary,
        sharedLibrary,
        test
    };

    struct Target
    {
        std::string name;
        TargetType type{TargetType::executable};
        std::filesystem::path entry;
        std::vector<std::filesystem::path> sources;
        std::vector<std::filesystem::path> includeDirectories;
        std::vector<std::string> dependencies;
    };
}
