#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace cippie
{
    enum class CommandType
    {
        help,
        version,
        build,
        run,
        test,
        clean,
        newProject,
        unknown
    };

    struct CommandLine
    {
        CommandType type{CommandType::help};
        std::string target;
        std::filesystem::path workingDirectory;
        std::vector<std::string> forwardedArguments;
        unsigned int jobs{0}; // 0 = default to hardware concurrency
        bool verbose{false};
        bool cleanCacheOnly{false};
        bool cleanAll{false};
    };

    class CommandLineParser
    {
    public:
        [[nodiscard]] CommandLine parse(
            int argc,
            char* argv[],
            const std::filesystem::path& workingDirectory
        ) const;
    };
}
