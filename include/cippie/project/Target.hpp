#pragma once

#include <cippie/project/TargetType.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace cippie
{
    struct Target
    {
        std::string name;
        TargetType type{TargetType::executable};

        std::optional<std::filesystem::path> entry;

        std::vector<std::string> sourcePatterns;
        std::vector<std::filesystem::path> includeDirectories;
        std::vector<std::filesystem::path> publicIncludeDirectories;

        std::vector<std::string> compileDefinitions;
        std::vector<std::string> compileOptions;
        std::vector<std::string> linkOptions;
        std::vector<std::string> links;

        // Expanded source files discovered from patterns or entry
        std::vector<std::filesystem::path> sources;
    };
}
