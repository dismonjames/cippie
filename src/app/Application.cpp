#include <cippie/app/Application.hpp>

#include <cippie/build/BuildEngine.hpp>
#include <cippie/build/BuildPlanner.hpp>
#include <cippie/config/ConfigLoader.hpp>
#include <cippie/core/ExitCode.hpp>
#include <cippie/core/Version.hpp>
#include <cippie/process/Process.hpp>
#include <cippie/project/ProjectLocator.hpp>
#include <cippie/project/TargetSelector.hpp>
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

        TargetSelector selector;
        auto selectedTargetRes = selector.select(project, commandLine.target);

        if (!selectedTargetRes.has_value())
        {
            logger_.error(selectedTargetRes.error().message);
            return toInt(ExitCode::configurationError);
        }

        const Target* selectedTarget = *selectedTargetRes;

        ToolchainDetector detector;
        const auto toolchain = detector.detect();

        BuildPlanner planner;
        const auto plan = planner.create(
            project,
            *selectedTarget,
            toolchain,
            "debug"
        );

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
        request.executable = plan.linkCommand.output;
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
