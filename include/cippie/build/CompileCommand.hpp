#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace cippie
{
    struct CompileCommand
    {
        std::filesystem::path compiler;
        std::filesystem::path source;
        std::filesystem::path object;
        std::vector<std::filesystem::path> includeDirectories;
        std::vector<std::string> definitions;
        std::vector<std::string> options;
    };
}
