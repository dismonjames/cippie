#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace cippie
{
    struct ArchiveCommand
    {
        std::filesystem::path archiver;
        std::vector<std::filesystem::path> objects;
        std::filesystem::path output;
        std::vector<std::string> options;
    };
}
