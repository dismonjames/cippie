#include <cippie/app/Application.hpp>

#include <cippie/build/BuildEngine.hpp>
#include <cippie/build/BuildPlanner.hpp>
#include <cippie/config/ConfigLoader.hpp>
#include <cippie/core/ExitCode.hpp>
#include <cippie/core/Version.hpp>
#include <cippie/process/Process.hpp>
#include <cippie/project/ProjectGenerator.hpp>
#include <cippie/project/ProjectLocator.hpp>
#include <cippie/project/TargetSelector.hpp>
#include <cippie/toolchain/ToolchainDetector.hpp>
#include <cippie/util/CleanRunner.hpp>

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
                return buildProject(commandLine);

            case CommandType::run:
                return runProject(commandLine);

            case CommandType::test:
                return testProject(commandLine);

            case CommandType::clean:
                return cleanProject(commandLine);

            case CommandType::newProject:
                return createNewProject(commandLine);

            case CommandType::unknown:
                logger_.error("unknown command");
                printHelp();
                return toInt(ExitCode::invalidArguments);
        }

        return toInt(ExitCode::generalError);
    }

    int Application::buildProject(const CommandLine& commandLine)
    {
        ProjectLocator locator;
        const auto projectRoot = locator.locate(commandLine.workingDirectory);

        if (!projectRoot)
        {
            logger_.error(
                "Cippiefile was not found in this directory or any parent directory"
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
        auto selectedTargetRes = selector.selectForBuild(project, commandLine.target);

        if (!selectedTargetRes.has_value())
        {
            logger_.error(selectedTargetRes.error().message);
            return toInt(ExitCode::configurationError);
        }

        const Target* selectedTarget = *selectedTargetRes;

        ToolchainDetector detector;
        const auto toolchain = detector.detect();

        BuildPlanner planner;
        const auto plan = planner.create(project, *selectedTarget, toolchain, "debug");

        BuildEngine engine(logger_);
        if (!engine.execute(plan, toolchain))
        {
            return toInt(ExitCode::buildFailed);
        }

        return toInt(ExitCode::success);
    }

    int Application::runProject(const CommandLine& commandLine)
    {
        ProjectLocator locator;
        const auto projectRoot = locator.locate(commandLine.workingDirectory);

        if (!projectRoot)
        {
            logger_.error(
                "Cippiefile was not found in this directory or any parent directory"
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
        auto selectedTargetRes = selector.selectForRun(project, commandLine.target);

        if (!selectedTargetRes.has_value())
        {
            logger_.error(selectedTargetRes.error().message);
            return toInt(ExitCode::configurationError);
        }

        const Target* selectedTarget = *selectedTargetRes;

        ToolchainDetector detector;
        const auto toolchain = detector.detect();

        BuildPlanner planner;
        const auto plan = planner.create(project, *selectedTarget, toolchain, "debug");

        BuildEngine engine(logger_);
        if (!engine.execute(plan, toolchain))
        {
            return toInt(ExitCode::buildFailed);
        }

        // Find root target plan artifact
        const TargetBuildPlan* rootPlan = nullptr;
        for (const auto& tPlan : plan.targetPlans)
        {
            if (tPlan.targetName == selectedTarget->name)
            {
                rootPlan = &tPlan;
                break;
            }
        }

        if (rootPlan == nullptr)
        {
            logger_.error("failed to find build plan for target: " + selectedTarget->name);
            return toInt(ExitCode::configurationError);
        }

        ProcessRequest request;
        request.executable = rootPlan->artifactOutput;
        request.arguments = commandLine.forwardedArguments;
        request.workingDirectory = project.rootDirectory;

        Process process;
        return process.run(request).exitCode;
    }

    int Application::testProject(const CommandLine& commandLine)
    {
        ProjectLocator locator;
        const auto projectRoot = locator.locate(commandLine.workingDirectory);

        if (!projectRoot)
        {
            logger_.error(
                "Cippiefile was not found in this directory or any parent directory"
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
        auto selectedTargetsRes = selector.selectForTest(project, commandLine.target);

        if (!selectedTargetsRes.has_value())
        {
            logger_.error(selectedTargetsRes.error().message);
            return toInt(ExitCode::configurationError);
        }

        const auto testTargets = *selectedTargetsRes;
        ToolchainDetector detector;
        const auto toolchain = detector.detect();
        BuildEngine engine(logger_);
        Process process;

        std::size_t passedCount = 0;
        std::size_t failedCount = 0;

        for (const auto* testTarget : testTargets)
        {
            BuildPlanner planner;
            const auto plan = planner.create(project, *testTarget, toolchain, "debug");

            if (!engine.execute(plan, toolchain))
            {
                logger_.error("build failed for test target: " + testTarget->name);
                return toInt(ExitCode::buildFailed);
            }

            const TargetBuildPlan* rootPlan = nullptr;
            for (const auto& tPlan : plan.targetPlans)
            {
                if (tPlan.targetName == testTarget->name)
                {
                    rootPlan = &tPlan;
                    break;
                }
            }

            if (rootPlan == nullptr)
            {
                continue;
            }

            ProcessRequest request;
            request.executable = rootPlan->artifactOutput;
            request.arguments = commandLine.forwardedArguments;
            request.workingDirectory = project.rootDirectory;

            logger_.info("[TEST] " + testTarget->name);
            auto res = process.run(request);

            if (res.exitCode == 0)
            {
                passedCount++;
            }
            else
            {
                failedCount++;
                logger_.error("test target '" + testTarget->name + "' failed with exit code " +
                              std::to_string(res.exitCode));
            }
        }

        std::cout << "\n";
        if (failedCount == 0)
        {
            std::cout << passedCount << " test target" << (passedCount == 1 ? "" : "s")
                      << " passed\n";
            return toInt(ExitCode::success);
        }

        std::cout << failedCount << " test target" << (failedCount == 1 ? "" : "s")
                  << " failed (" << passedCount << " passed)\n";
        return 7; // ExitCode for test failure (7 per spec Section 32)
    }

    int Application::cleanProject(const CommandLine& commandLine)
    {
        ProjectLocator locator;
        const auto projectRoot = locator.locate(commandLine.workingDirectory);

        if (!projectRoot)
        {
            logger_.error(
                "Cippiefile was not found in this directory or any parent directory"
            );
            return toInt(ExitCode::projectNotFound);
        }

        CleanRunner cleanRunner(logger_);
        auto res = cleanRunner.clean(*projectRoot);

        if (!res.has_value())
        {
            logger_.error(res.error().message);
            return toInt(ExitCode::generalError);
        }

        return toInt(ExitCode::success);
    }

    int Application::createNewProject(const CommandLine& commandLine)
    {
        if (commandLine.target.empty())
        {
            logger_.error("missing project name for 'cippie new'");
            return toInt(ExitCode::invalidArguments);
        }

        ProjectGenerator generator;
        auto genRes = generator.generate(commandLine.target, commandLine.workingDirectory);

        if (!genRes.has_value())
        {
            logger_.error(genRes.error().message);
            return toInt(ExitCode::configurationError);
        }

        logger_.info("Created project '" + commandLine.target + "' at " + genRes->string());
        return toInt(ExitCode::success);
    }

    void Application::printHelp() const
    {
        std::cout
            << "Cippie - C++ build system and package manager\n\n"
            << "Usage:\n"
            << "  cippie <command> [target] [-- arguments]\n\n"
            << "Commands:\n"
            << "  new <name>  Create a new Cippie project\n"
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
