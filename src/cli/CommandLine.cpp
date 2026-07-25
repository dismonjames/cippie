#include <cippie/cli/CommandLine.hpp>

#include <charconv>
#include <string_view>

namespace cippie
{
    namespace
    {
        [[nodiscard]] CommandType parseCommandType(std::string_view value)
        {
            if (value == "help" || value == "--help" || value == "-h") return CommandType::help;
            if (value == "version" || value == "--version") return CommandType::version;
            if (value == "build") return CommandType::build;
            if (value == "run") return CommandType::run;
            if (value == "test") return CommandType::test;
            if (value == "clean") return CommandType::clean;
            if (value == "new") return CommandType::newProject;
            if (value == "add") return CommandType::add;
            if (value == "remove") return CommandType::remove;
            if (value == "restore") return CommandType::restore;
            if (value == "doctor") return CommandType::doctor;
            if (value == "update") return CommandType::update;
            return CommandType::unknown;
        }

        bool parseJobCount(std::string_view val, unsigned int& outJobs)
        {
            if (val.empty()) return false;
            unsigned int parsed = 0;
            auto res = std::from_chars(val.data(), val.data() + val.size(), parsed);
            if (res.ec != std::errc() || res.ptr != val.data() + val.size() || parsed == 0)
                return false;
            outJobs = std::min(parsed, 128u);
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

        if (argc < 2) return result;

        result.type = parseCommandType(argv[1]);
        if (result.type == CommandType::unknown)
        {
            return result;
        }

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

            if (argument == "-v" || argument == "--verbose") { result.verbose = true; continue; }
            if (argument == "-q" || argument == "--quiet") { result.quiet = true; continue; }
            if (argument == "--cache") { result.cleanCacheOnly = true; continue; }
            if (argument == "--all") { result.cleanAll = true; continue; }
            if (argument == "--force" || argument == "-f") { result.force = true; continue; }
            if (argument == "--offline") { result.offline = true; continue; }
            if (argument == "--locked") { result.locked = true; continue; }
            if (argument == "--debug") { result.configuration = "debug"; continue; }
            if (argument == "--release") { result.configuration = "release"; continue; }

            if (argument == "-j" || argument == "--jobs")
            {
                if (index + 1 < argc)
                {
                    ++index;
                    if (!parseJobCount(argv[index], result.jobs))
                        result.type = CommandType::unknown;
                }
                else
                {
                    result.type = CommandType::unknown;
                }
                continue;
            }

            if (argument.starts_with("-j"))
            {
                if (!parseJobCount(argument.substr(2), result.jobs))
                    result.type = CommandType::unknown;
                continue;
            }
            if (argument.starts_with("--jobs="))
            {
                if (!parseJobCount(argument.substr(7), result.jobs))
                    result.type = CommandType::unknown;
                continue;
            }

            // --target <triple>
            if (argument == "--target")
            {
                if (index + 1 < argc)
                {
                    ++index;
                    result.targetTriple = std::string(argv[index]);
                }
                else
                {
                    result.type = CommandType::unknown;
                }
                continue;
            }
            if (argument.starts_with("--target="))
            {
                result.targetTriple = std::string(argument.substr(9));
                continue;
            }

            // --toolchain <name>
            if (argument == "--toolchain")
            {
                if (index + 1 < argc)
                {
                    ++index;
                    result.toolchainName = std::string(argv[index]);
                }
                else
                {
                    result.type = CommandType::unknown;
                }
                continue;
            }
            if (argument.starts_with("--toolchain="))
            {
                result.toolchainName = std::string(argument.substr(12));
                continue;
            }

            // Any other flag starting with '-' is unrecognized option
            if (argument.starts_with("-"))
            {
                result.type = CommandType::unknown;
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
