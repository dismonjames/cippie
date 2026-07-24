#include <cippie/cli/CommandLine.hpp>

#include <charconv>
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

            if (value == "add")
            {
                return CommandType::add;
            }

            if (value == "remove")
            {
                return CommandType::remove;
            }

            if (value == "restore")
            {
                return CommandType::restore;
            }

            return CommandType::unknown;
        }

        bool parseJobCount(std::string_view val, unsigned int& outJobs)
        {
            if (val.empty()) return false;
            unsigned int parsed = 0;
            auto res = std::from_chars(val.data(), val.data() + val.size(), parsed);
            if (res.ec != std::errc() || res.ptr != val.data() + val.size() || parsed == 0)
            {
                return false;
            }
            if (parsed > 128)
            {
                parsed = 128;
            }
            outJobs = parsed;
            return true;
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

            if (argument == "-v" || argument == "--verbose")
            {
                result.verbose = true;
                continue;
            }

            if (argument == "--cache")
            {
                result.cleanCacheOnly = true;
                continue;
            }

            if (argument == "--all")
            {
                result.cleanAll = true;
                continue;
            }

            if (argument == "--offline")
            {
                result.offline = true;
                continue;
            }

            if (argument == "--locked")
            {
                result.locked = true;
                continue;
            }

            if (argument == "-j" || argument == "--jobs")
            {
                if (index + 1 < argc)
                {
                    ++index;
                    if (!parseJobCount(argv[index], result.jobs))
                    {
                        result.type = CommandType::unknown;
                    }
                }
                else
                {
                    result.type = CommandType::unknown;
                }
                continue;
            }

            if (argument.starts_with("-j"))
            {
                std::string_view numStr = argument.substr(2);
                if (!parseJobCount(numStr, result.jobs))
                {
                    result.type = CommandType::unknown;
                }
                continue;
            }

            if (argument.starts_with("--jobs="))
            {
                std::string_view numStr = argument.substr(7);
                if (!parseJobCount(numStr, result.jobs))
                {
                    result.type = CommandType::unknown;
                }
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
