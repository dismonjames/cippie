#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace cippie
{
    struct ProcessRequest
    {
        std::string executable;
        std::vector<std::string> arguments;
        std::filesystem::path workingDirectory;
    };

    struct ProcessResult
    {
        int exitCode{0};
        bool exitedNormally{true};
    };

    class Process
    {
    public:
        [[nodiscard]] ProcessResult run(
            const ProcessRequest& request
        ) const;
    };
}
