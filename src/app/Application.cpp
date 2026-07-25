#include <cippie/app/Application.hpp>

#include <cippie/build/BuildEngine.hpp>
#include <cippie/build/BuildPlanner.hpp>
#include <cippie/config/CippiefileEditor.hpp>
#include <cippie/config/ConfigLoader.hpp>
#include <cippie/core/ExitCode.hpp>
#include <cippie/core/Version.hpp>
#include <cippie/package/DependencyResolver.hpp>
#include <cippie/package/LockFile.hpp>
#include <cippie/process/Process.hpp>
#include <cippie/project/ProjectGenerator.hpp>
#include <cippie/project/ProjectLocator.hpp>
#include <cippie/project/TargetSelector.hpp>
#include <cippie/toolchain/ToolchainDetector.hpp>
#include <cippie/toolchain/ToolchainRegistry.hpp>
#include <cippie/util/BuildLock.hpp>
#include <cippie/util/CleanRunner.hpp>

#include <fstream>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <thread>

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

            case CommandType::restore:
                return restoreProject(commandLine);

            case CommandType::add:
                return addPackage(commandLine);

            case CommandType::remove:
                return removePackage(commandLine);

            case CommandType::doctor:
                return runDoctor(commandLine);

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

        auto lockRes = BuildLock::acquire(*projectRoot);
        if (!lockRes.has_value())
        {
            logger_.error(lockRes.error().message);
            return toInt(ExitCode::generalError);
        }

        ConfigLoader configLoader;
        auto projectResult = configLoader.load(*projectRoot);

        if (!projectResult.has_value())
        {
            logger_.error(projectResult.error().message);
            return toInt(ExitCode::configurationError);
        }

        const auto project = std::move(*projectResult);

        // Auto-restore dependencies if declared
        if (!project.dependencies.empty())
        {
            if (restoreProject(commandLine) != toInt(ExitCode::success))
            {
                return toInt(ExitCode::buildFailed);
            }
        }

        TargetSelector selector;
        auto selectedTargetRes = selector.selectForBuild(project, commandLine.target);

        if (!selectedTargetRes.has_value())
        {
            logger_.error(selectedTargetRes.error().message);
            return toInt(ExitCode::configurationError);
        }

        const Target* selectedTarget = *selectedTargetRes;

        DetectOptions detectOpts{
            .toolchainName = commandLine.toolchainName.empty()
                ? std::optional<std::string>{}
                : std::optional<std::string>(commandLine.toolchainName),
            .targetTripleStr = commandLine.targetTriple
        };

        ToolchainDetector detector;
        auto toolchainRes = detector.detect(detectOpts);
        if (!toolchainRes.has_value())
        {
            logger_.error(toolchainRes.error().message);
            return toInt(ExitCode::buildFailed);
        }
        const auto toolchain = std::move(*toolchainRes);

        BuildPlanner planner;
        const auto plan = planner.create(project, *selectedTarget, toolchain, "debug");

        BuildEngine engine(logger_);
        if (!engine.execute(plan, toolchain, commandLine.jobs, commandLine.verbose))
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

        auto lockRes = BuildLock::acquire(*projectRoot);
        if (!lockRes.has_value())
        {
            logger_.error(lockRes.error().message);
            return toInt(ExitCode::generalError);
        }

        ConfigLoader configLoader;
        auto projectResult = configLoader.load(*projectRoot);

        if (!projectResult.has_value())
        {
            logger_.error(projectResult.error().message);
            return toInt(ExitCode::configurationError);
        }

        const auto project = std::move(*projectResult);

        if (!project.dependencies.empty())
        {
            if (restoreProject(commandLine) != toInt(ExitCode::success))
            {
                return toInt(ExitCode::buildFailed);
            }
        }

        TargetSelector selector;
        auto selectedTargetRes = selector.selectForRun(project, commandLine.target);

        if (!selectedTargetRes.has_value())
        {
            logger_.error(selectedTargetRes.error().message);
            return toInt(ExitCode::configurationError);
        }

        const Target* selectedTarget = *selectedTargetRes;

        DetectOptions detectOpts{
            .toolchainName = commandLine.toolchainName.empty()
                ? std::optional<std::string>{}
                : std::optional<std::string>(commandLine.toolchainName),
            .targetTripleStr = commandLine.targetTriple
        };

        ToolchainDetector detector;
        auto toolchainRes = detector.detect(detectOpts);
        if (!toolchainRes.has_value())
        {
            logger_.error(toolchainRes.error().message);
            return toInt(ExitCode::buildFailed);
        }
        const auto toolchain = std::move(*toolchainRes);

        // Cross-compilation: refuse to run non-native target
        if (!toolchain.target.isNativeRunnable(toolchain.host))
        {
            logger_.error(
                "cannot run target binary for '" + toolchain.target.toString() +
                "' on host '" + toolchain.host.toString() + "'; use 'cippie build' instead"
            );
            return toInt(ExitCode::generalError);
        }

        BuildPlanner planner;
        const auto plan = planner.create(project, *selectedTarget, toolchain, "debug");

        BuildEngine engine(logger_);
        if (!engine.execute(plan, toolchain, commandLine.jobs, commandLine.verbose))
        {
            return toInt(ExitCode::buildFailed);
        }

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

        auto lockRes = BuildLock::acquire(*projectRoot);
        if (!lockRes.has_value())
        {
            logger_.error(lockRes.error().message);
            return toInt(ExitCode::generalError);
        }

        ConfigLoader configLoader;
        auto projectResult = configLoader.load(*projectRoot);

        if (!projectResult.has_value())
        {
            logger_.error(projectResult.error().message);
            return toInt(ExitCode::configurationError);
        }

        const auto project = std::move(*projectResult);

        if (!project.dependencies.empty())
        {
            if (restoreProject(commandLine) != toInt(ExitCode::success))
            {
                return toInt(ExitCode::buildFailed);
            }
        }

        TargetSelector selector;
        auto selectedTargetsRes = selector.selectForTest(project, commandLine.target);

        if (!selectedTargetsRes.has_value())
        {
            logger_.error(selectedTargetsRes.error().message);
            return toInt(ExitCode::configurationError);
        }

        const auto testTargets = *selectedTargetsRes;

        DetectOptions detectOpts{
            .toolchainName = commandLine.toolchainName.empty()
                ? std::optional<std::string>{}
                : std::optional<std::string>(commandLine.toolchainName),
            .targetTripleStr = commandLine.targetTriple
        };

        ToolchainDetector detector;
        auto toolchainRes = detector.detect(detectOpts);
        if (!toolchainRes.has_value())
        {
            logger_.error(toolchainRes.error().message);
            return toInt(ExitCode::buildFailed);
        }
        const auto toolchain = std::move(*toolchainRes);

        // Cross-compiled tests cannot run on host
        if (!toolchain.target.isNativeRunnable(toolchain.host))
        {
            logger_.error(
                "cannot run tests for cross target '" + toolchain.target.toString() +
                "' on host '" + toolchain.host.toString() + "'"
            );
            return toInt(ExitCode::generalError);
        }

        BuildEngine engine(logger_);
        Process process;

        std::size_t passedCount = 0;
        std::size_t failedCount = 0;

        for (const auto* testTarget : testTargets)
        {
            BuildPlanner planner;
            const auto plan = planner.create(project, *testTarget, toolchain, "debug");

            if (!engine.execute(plan, toolchain, commandLine.jobs, commandLine.verbose))
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
        return 7;
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
        auto res = cleanRunner.clean(*projectRoot, commandLine.cleanCacheOnly, commandLine.cleanAll);

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

    int Application::restoreProject(const CommandLine& commandLine)
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

        std::optional<LockFile> existingLock;
        const auto lockPath = *projectRoot / "Cippie.lock";
        if (std::filesystem::exists(lockPath))
        {
            auto lockLoadRes = LockFile::load(lockPath);
            if (lockLoadRes.has_value())
            {
                existingLock = std::move(*lockLoadRes);
            }
        }

        DependencyResolver resolver(PackageRegistry(), PackageCache::getCacheDirectory());
        auto graphRes = resolver.resolve(project, existingLock, commandLine.offline, commandLine.locked);

        if (!graphRes.has_value())
        {
            logger_.error(graphRes.error().message);
            return toInt(ExitCode::configurationError);
        }

        if (!commandLine.locked)
        {
            LockFile newLock;
            for (const auto& [name, rPkg] : graphRes->packages)
            {
                newLock.addPackage(LockedPackage{
                    .name = name,
                    .version = rPkg.version,
                    .sourceType = rPkg.sourceType,
                    .sourceLocation = rPkg.sourceLocation,
                    .commit = rPkg.commit,
                    .integrity = rPkg.integrity,
                    .dependencies = rPkg.dependencies
                });
            }
            (void)newLock.save(lockPath);
        }

        logger_.info("Restore complete.");
        return toInt(ExitCode::success);
    }

    int Application::addPackage(const CommandLine& commandLine)
    {
        if (commandLine.target.empty())
        {
            logger_.error("missing package name for 'cippie add'");
            return toInt(ExitCode::invalidArguments);
        }

        ProjectLocator locator;
        const auto projectRoot = locator.locate(commandLine.workingDirectory);

        if (!projectRoot)
        {
            logger_.error(
                "Cippiefile was not found in this directory or any parent directory"
            );
            return toInt(ExitCode::projectNotFound);
        }

        const auto cippiefilePath = *projectRoot / "Cippiefile";

        // Read backup for rollback
        std::ifstream file(cippiefilePath);
        std::string backup((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        std::string pkgStr = commandLine.target;
        std::string name = pkgStr;
        std::string verReq = "*";

        auto atPos = pkgStr.find('@');
        if (atPos != std::string::npos)
        {
            name = pkgStr.substr(0, atPos);
            verReq = pkgStr.substr(atPos + 1);
        }

        std::string expr = "package(\"" + name + "\", \"" + verReq + "\")";

        auto editRes = CippiefileEditor::addDependency(cippiefilePath, expr);
        if (!editRes.has_value())
        {
            logger_.error(editRes.error().message);
            return toInt(ExitCode::generalError);
        }

        int restoreCode = restoreProject(commandLine);
        if (restoreCode != toInt(ExitCode::success))
        {
            // Rollback Cippiefile edit
            std::ofstream outFile(cippiefilePath);
            outFile << backup;
            logger_.error("rolled back Cippiefile edit due to resolution failure");
            return restoreCode;
        }

        return toInt(ExitCode::success);
    }

    int Application::removePackage(const CommandLine& commandLine)
    {
        if (commandLine.target.empty())
        {
            logger_.error("missing package name for 'cippie remove'");
            return toInt(ExitCode::invalidArguments);
        }

        ProjectLocator locator;
        const auto projectRoot = locator.locate(commandLine.workingDirectory);

        if (!projectRoot)
        {
            logger_.error(
                "Cippiefile was not found in this directory or any parent directory"
            );
            return toInt(ExitCode::projectNotFound);
        }

        const auto cippiefilePath = *projectRoot / "Cippiefile";
        auto editRes = CippiefileEditor::removeDependency(cippiefilePath, commandLine.target);

        if (!editRes.has_value())
        {
            logger_.error(editRes.error().message);
            return toInt(ExitCode::generalError);
        }

        return restoreProject(commandLine);
    }

    int Application::runDoctor(const CommandLine& commandLine)
    {
        std::cout << "Cippie Doctor\n";
        std::cout << "=============\n";

        const auto host = TargetTriple::detectHost();
        std::cout << "Host triple      : " << host.toString() << "\n";

        DetectOptions detectOpts{
            .toolchainName = commandLine.toolchainName.empty()
                ? std::optional<std::string>{}
                : std::optional<std::string>(commandLine.toolchainName),
            .targetTripleStr = commandLine.targetTriple
        };

        TargetTriple targetTriple = host;
        if (commandLine.targetTriple.has_value())
        {
            auto tripleRes = TargetTriple::parse(*commandLine.targetTriple);
            if (tripleRes.has_value()) targetTriple = *tripleRes;
        }
        std::cout << "Target triple    : " << targetTriple.toString() << "\n";

        ToolchainDetector detector;
        auto toolchainRes = detector.detect(detectOpts);

        if (!toolchainRes.has_value())
        {
            std::cout << "Toolchain        : [NOT FOUND] " << toolchainRes.error().message << "\n";
        }
        else
        {
            const auto& tc = *toolchainRes;
            std::cout << "Toolchain name   : " << tc.name << "\n";
            std::cout << "Compiler family  : " << toString(tc.compilerFamily) << "\n";
            std::cout << "Compiler (CXX)   : " << tc.cxxCompiler.string();
            if (!tc.version.empty() && tc.version != "unknown")
            {
                std::cout << " (" << tc.version << ")";
            }
            std::cout << "\n";
            std::cout << "Compiler (CC)    : " << tc.cCompiler.string() << "\n";
            std::cout << "Linker           : " << tc.linker.string() << "\n";
            std::cout << "Archiver         : " << tc.archiver.string() << "\n";

            if (!tc.sysroot.empty())
            {
                std::cout << "Sysroot          : " << tc.sysroot.string() << "\n";
            }
            else
            {
                std::cout << "Sysroot          : (none)\n";
            }
        }

        // Parallel jobs
        const unsigned int hwThreads = std::thread::hardware_concurrency();
        std::cout << "Parallel jobs    : " << (commandLine.jobs > 0 ? commandLine.jobs : hwThreads) << "\n";

        // Project cache
        ProjectLocator locator;
        const auto projectRoot = locator.locate(commandLine.workingDirectory);
        if (projectRoot)
        {
            std::cout << "Project cache    : " << (*projectRoot / ".cippie").string() << "\n";
        }
        else
        {
            std::cout << "Project cache    : (no Cippiefile found)\n";
        }

        // Package cache
        std::cout << "Package cache    : " << PackageCache::getCacheDirectory().string() << "\n";

        // Toolchain config dir
        std::cout << "Toolchain configs: " << ToolchainRegistry::getConfigDir().string() << "\n";

        // Registered cross-toolchains
        auto regRes = ToolchainRegistry::load();
        if (regRes.has_value() && !regRes->toolchains().empty())
        {
            std::cout << "Cross-toolchains :\n";
            for (const auto& tc : regRes->toolchains())
            {
                std::cout << "  " << tc.name << " -> " << tc.target.toString()
                          << " (" << tc.cxxCompiler.string() << ")\n";
            }
        }
        else
        {
            std::cout << "Cross-toolchains : (none registered)\n";
        }

        // Missing tools check
        std::cout << "Missing tools    :\n";
        bool anyMissing = false;
        auto checkTool = [&](const std::string& name, const std::filesystem::path& path) {
            std::error_code ec;
            if (!std::filesystem::exists(path, ec))
            {
                std::cout << "  " << name << ": not found (" << path.string() << ")\n";
                anyMissing = true;
            }
        };

        if (toolchainRes.has_value())
        {
            const auto& tc = *toolchainRes;
            checkTool("CXX", tc.cxxCompiler);
            checkTool("AR", tc.archiver);
        }

        if (!anyMissing)
        {
            std::cout << "  (none)\n";
        }

        return toInt(ExitCode::success);
    }

    void Application::printHelp() const
    {
        std::cout
            << "Cippie - C++ build system and package manager\n\n"
            << "Usage:\n"
            << "  cippie <command> [target] [options] [-- arguments]\n\n"
            << "Commands:\n"
            << "  new <name>       Create a new Cippie project\n"
            << "  build            Build a target\n"
            << "  run              Build and run a target\n"
            << "  test             Build and run tests\n"
            << "  clean            Remove generated build files (--cache / --all)\n"
            << "  add <pkg[@ver]>  Add a package dependency\n"
            << "  remove <pkg>     Remove a package dependency\n"
            << "  restore          Restore project dependencies (--offline / --locked)\n"
            << "  doctor           Show toolchain and environment diagnostics\n"
            << "  help             Show this help\n"
            << "  version          Show the Cippie version\n\n"
            << "Options:\n"
            << "  -j, --jobs N         Number of parallel build workers\n"
            << "  -v, --verbose        Verbose build output\n"
            << "  --target <triple>    Cross-compile for target triple (e.g. aarch64-linux-gnu)\n"
            << "  --toolchain <name>   Use named toolchain from ~/.config/cippie/toolchains/\n"
            << "  --offline            Prohibit network operations\n"
            << "  --locked             Refuse updating lock file\n";
    }

    void Application::printVersion() const
    {
        std::cout << "Cippie " << version() << '\n';
    }
}
