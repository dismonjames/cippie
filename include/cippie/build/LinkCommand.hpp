#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace cippie
{
    struct LinkCommand
    {
        std::filesystem::path linker;
        std::vector<std::filesystem::path> objects;
        std::filesystem::path output;
        std::vector<std::string> options;
        std::vector<std::string> libraries;
    };
}
