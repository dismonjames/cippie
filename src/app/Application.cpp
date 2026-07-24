#include <cippie/app/Application.hpp>

#include <cippie/build/BuildEngine.hpp>
#include <cippie/build/BuildPlanner.hpp>
#include <cippie/config/ConfigLoader.hpp>
#include <cippie/core/ExitCode.hpp>
#include <cippie/core/Version.hpp>
#include <cippie/process/Process.hpp>
#include <cippie/project/ProjectLocator.hpp>
#include <cippie/toolchain/ToolchainDetector.hpp>

#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace cippie
{
    int Application::run(int argc, char* argv[])
    {
        try
        {
            CommandLineParser parser;
            const auto commandLine = parser.parse(
                argc,
                argv,
                std::filesystem::current_path()
            );

            return dispatch(commandLine);
        }
        catch (const std::exception& exception)
        {
            logger_.error(exception.what());
            return toInt(ExitCode::generalError);
        }
    }

    int Application::dispatch(const CommandLine& commandLine)
    {
        switch (commandLine.type)
        {
            case CommandType::help:
                printHelp();
                return toInt(ExitCode::success);

            case CommandType::version:
                printVersion();
                return toInt(ExitCode::success);

            case CommandType::build:
                return buildProject(commandLine, false);

            case CommandType::run:
                return buildProject(commandLine, true);

            case CommandType::test:
                logger_.warning("test command is not implemented yet");
                return toInt(ExitCode::generalError);

            case CommandType::clean:
                logger_.warning("clean command is not implemented yet");
                return toInt(ExitCode::generalError);

            case CommandType::unknown:
                logger_.error("unknown command");
                printHelp();
                return toInt(ExitCode::invalidArguments);
        }

        return toInt(ExitCode::generalError);
    }

    int Application::buildProject(
        const CommandLine& commandLine,
        bool runAfterBuild
    )
    {
        ProjectLocator locator;
        const auto projectRoot = locator.locate(
            commandLine.workingDirectory
        );

        if (!projectRoot)
        {
            logger_.error(
                "Cippiefile was not found in this directory "
                "or any parent directory"
            );

            return toInt(ExitCode::projectNotFound);
        }

        ConfigLoader configLoader;
        auto projectResult = configLoader.load(*projectRoot);

        if (!projectResult.has_value())
        {
            logger_.error(projectResult.error().message);
            return toInt(ExitCode::configurationError);
        }

        const auto project = std::move(*projectResult);

        if (project.targets.empty())
        {
            logger_.error("project has no build targets");
            return toInt(ExitCode::configurationError);
        }

        const Target* selectedTarget = nullptr;

        if (commandLine.target.empty())
        {
            selectedTarget = &project.targets.front();
        }
        else
        {
            for (const auto& target : project.targets)
            {
                if (target.name == commandLine.target)
                {
                    selectedTarget = &target;
                    break;
                }
            }
        }

        if (selectedTarget == nullptr)
        {
            logger_.error("target not found: " + commandLine.target);
            return toInt(ExitCode::configurationError);
        }

        BuildPlanner planner;
        const auto plan = planner.create(
            project,
            *selectedTarget,
            "debug"
        );

        ToolchainDetector detector;
        const auto toolchain = detector.detect();
        BuildEngine engine(logger_);

        if (!engine.execute(plan, toolchain))
        {
            return toInt(ExitCode::buildFailed);
        }

        if (!runAfterBuild)
        {
            return toInt(ExitCode::success);
        }

        ProcessRequest request;
        request.executable = plan.linkStep.output.string();
        request.arguments = commandLine.forwardedArguments;
        request.workingDirectory = project.rootDirectory;

        Process process;
        return process.run(request).exitCode;
    }

    void Application::printHelp() const
    {
        std::cout
            << "Cippie - C++ build system and package manager\n\n"
            << "Usage:\n"
            << "  cippie <command> [target] [-- arguments]\n\n"
            << "Commands:\n"
            << "  build       Build a target\n"
            << "  run         Build and run a target\n"
            << "  test        Build and run tests\n"
            << "  clean       Remove generated build files\n"
            << "  help        Show this help\n"
            << "  version     Show the Cippie version\n";
    }

    void Application::printVersion() const
    {
        std::cout << "Cippie " << version() << '\n';
    }
}
