#include <cippie/cli/CommandLine.hpp>

#include <string_view>

namespace cippie
{
    namespace
    {
        [[nodiscard]] CommandType parseCommandType(std::string_view value)
        {
            if (value == "help" || value == "--help" || value == "-h")
            {
                return CommandType::help;
            }

            if (value == "version" || value == "--version" || value == "-v")
            {
                return CommandType::version;
            }

            if (value == "build")
            {
                return CommandType::build;
            }

            if (value == "run")
            {
                return CommandType::run;
            }

            if (value == "test")
            {
                return CommandType::test;
            }

            if (value == "clean")
            {
                return CommandType::clean;
            }

            if (value == "new")
            {
                return CommandType::newProject;
            }

            return CommandType::unknown;
        }
    }

    CommandLine CommandLineParser::parse(
        int argc,
        char* argv[],
        const std::filesystem::path& workingDirectory
    ) const
    {
        CommandLine result;
        result.workingDirectory = workingDirectory;

        if (argc < 2)
        {
            return result;
        }

        result.type = parseCommandType(argv[1]);
        bool forwardingArguments = false;

        for (int index = 2; index < argc; ++index)
        {
            const std::string_view argument = argv[index];

            if (argument == "--")
            {
                forwardingArguments = true;
                continue;
            }

            if (forwardingArguments)
            {
                result.forwardedArguments.emplace_back(argument);
                continue;
            }

            if (result.target.empty())
            {
                result.target = argument;
            }
            else
            {
                result.forwardedArguments.emplace_back(argument);
            }
        }

        return result;
    }
}
