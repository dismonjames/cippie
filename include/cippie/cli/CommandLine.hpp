#pragma once

#include <filesystem>
#include <optional>
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
        add,
        remove,
        restore,
        doctor,
        unknown
    };

    struct CommandLine
    {
        CommandType type{CommandType::help};
        std::string target;                         // positional target name
        std::optional<std::string> targetTriple;    // --target <triple>
        std::string toolchainName;                  // --toolchain <name>
        std::string configuration{"debug"};         // --debug / --release
        std::filesystem::path workingDirectory;
        std::vector<std::string> forwardedArguments;
        unsigned int jobs{0};
        bool verbose{false};
        bool quiet{false};
        bool cleanCacheOnly{false};
        bool cleanAll{false};
        bool offline{false};
        bool locked{false};
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
