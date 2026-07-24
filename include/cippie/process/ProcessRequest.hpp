#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace cippie
{
    struct ProcessRequest
    {
        std::filesystem::path executable;
        std::vector<std::string> arguments;
        std::filesystem::path workingDirectory;
        bool captureOutput{false};
    };
}
