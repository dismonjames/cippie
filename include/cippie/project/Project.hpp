#pragma once

#include <cippie/project/Target.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace cippie
{
    struct Project
    {
        std::string name;
        int cppStandard{23};
        std::filesystem::path rootDirectory;
        std::filesystem::path configurationFile;
        std::vector<Target> targets;
    };
}
