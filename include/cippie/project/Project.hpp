#pragma once

#include <cippie/project/BuildConfiguration.hpp>
#include <cippie/project/Dependency.hpp>
#include <cippie/project/Target.hpp>

#include <filesystem>
#include <optional>
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
        std::optional<std::string> defaultTarget;
        std::vector<BuildConfiguration> configurations;
        std::vector<Dependency> dependencies;
        std::vector<Target> targets;
    };
}
