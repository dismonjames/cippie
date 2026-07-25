#include <cippie/build/BuildPlanner.hpp>
#include <cippie/build/BuildGraph.hpp>
#include <cippie/build/SourceScanner.hpp>
#include <cippie/config/ConfigLoader.hpp>
#include <cippie/package/DependencyResolver.hpp>

#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace cippie
{
    namespace
    {
        void addUniquePath(
            std::vector<std::filesystem::path>& container,
            const std::filesystem::path& path
        )
        {
            if (std::find(container.begin(), container.end(), path) == container.end())
            {
                container.push_back(path);
            }
        }

        void translateOptions(std::vector<std::string>& options, CompilerFamily family)
        {
            if (family != CompilerFamily::msvc) return;

            std::vector<std::string> translated;
            for (auto& opt : options)
            {
                if (opt == "-Wall" || opt == "-Wextra" || opt == "-Wpedantic" || opt == "-Wconversion" || opt == "-Wshadow")
                {
                    translated.push_back("/W4");
                }
                else if (opt.rfind("-std=c++", 0) == 0)
                {
                    auto standard = opt.substr(9);
                    if (standard == "23")
                        translated.push_back("/std:c++latest");
                    else
                        translated.push_back(opt);
                }
                else if (opt == "-g")
                {
                    translated.push_back("/Zi");
                }
                else if (opt == "-fPIC" || opt.rfind("-Wl,", 0) == 0)
                {
                    // not applicable on Windows
                }
                else
                {
                    translated.push_back(opt);
                }
            }
            options = std::move(translated);
        }
    }

    BuildPlan BuildPlanner::create(
        const Project& project,
        const Target& target,
        const Toolchain& toolchain,
        std::string_view configuration
    ) const
    {
        auto graphRes = BuildGraph::fromProject(project);
        if (!graphRes.has_value())
        {
            throw std::runtime_error(graphRes.error().message);
        }

        const auto& graph = *graphRes;
        auto execPlanRes = graph.getExecutionPlan(target);
        if (!execPlanRes.has_value())
        {
            throw std::runtime_error(execPlanRes.error().message);
        }

        const auto& targetsToBuild = *execPlanRes;

        std::unordered_map<std::string, const Target*> targetMap;
        for (const auto& t : project.targets)
        {
            targetMap[t.name] = &t;
        }

        std::unordered_map<std::string, std::filesystem::path> artifactMap;
        const std::string tripleStr = toolchain.target.toString();

        BuildPlan plan;
        plan.rootTargetName = target.name;

        // Resolve package dependencies if declared
        ResolvedPackageGraph packageGraph;
        ConfigLoader configLoader;

        if (!project.dependencies.empty())
        {
            DependencyResolver resolver(PackageRegistry(), PackageCache::getCacheDirectory());
            auto resolveRes = resolver.resolve(project);
            if (resolveRes.has_value())
            {
                packageGraph = std::move(*resolveRes);
            }
        }

        // Add package plans to execution graph
        for (const auto& [pkgName, rPkg] : packageGraph.packages)
        {
            auto pkgProjRes = configLoader.load(rPkg.sourceDirectory);
            if (!pkgProjRes.has_value()) continue;

            const auto& pkgProj = *pkgProjRes;
            for (const auto& pkgTarget : pkgProj.targets)
            {
                SourceScanner scanner;
                auto sources = scanner.scan(
                    pkgProj.rootDirectory,
                    pkgTarget.sourcePatterns,
                    pkgTarget.entry
                );

                if (sources.empty()) continue;

                const std::string namespacedName = "package:" + pkgName + "::" + pkgTarget.name;

                const auto targetBuildDirectory =
                    project.rootDirectory /
                    ".cippie/build" /
                    tripleStr /
                    std::string(configuration) /
                    pkgName;

                TargetBuildPlan targetPlan;
                targetPlan.targetName = namespacedName;
                targetPlan.targetType = pkgTarget.type;

                if (pkgTarget.type == TargetType::staticLibrary)
                {
                    targetPlan.artifactOutput = targetBuildDirectory / "lib" /
                        (toolchain.compilerFamily == CompilerFamily::msvc ? pkgName + ".lib" : "lib" + pkgName + ".a");
                }
                else if (pkgTarget.type == TargetType::sharedLibrary)
                {
                    targetPlan.artifactOutput = targetBuildDirectory / "lib" /
                        (toolchain.compilerFamily == CompilerFamily::msvc ? pkgName + ".dll" : "lib" + pkgName + ".so");
                }
                else
                {
                    auto exeName = pkgName;
                    if (toolchain.compilerFamily == CompilerFamily::msvc && exeName.find('.') == std::string::npos)
                        exeName += ".exe";
                    targetPlan.artifactOutput = targetBuildDirectory / "bin" / exeName;
                }

                artifactMap[pkgName] = targetPlan.artifactOutput;
                artifactMap[namespacedName] = targetPlan.artifactOutput;

                for (const auto& source : sources)
                {
                    std::filesystem::path relSource = source.filename();
                    auto object = targetBuildDirectory / "obj" / relSource;
                    object += (toolchain.compilerFamily == CompilerFamily::msvc) ? ".obj" : ".o";

                    CompileCommand cmd;
                    cmd.compiler = toolchain.cxxCompiler;
                    cmd.source = source;
                    cmd.object = object;

                    for (const auto& inc : pkgTarget.includeDirectories)
                    {
                        addUniquePath(cmd.includeDirectories, inc);
                    }
                    for (const auto& pubInc : pkgTarget.publicIncludeDirectories)
                    {
                        addUniquePath(cmd.includeDirectories, pubInc);
                    }

                    cmd.definitions = pkgTarget.compileDefinitions;
                    cmd.options = {"-std=c++" + std::to_string(pkgProj.cppStandard), "-Wall", "-Wextra", "-g"};

                    if (pkgTarget.type == TargetType::sharedLibrary)
                    {
                        cmd.options.push_back("-fPIC");
                    }

                    translateOptions(cmd.options, toolchain.compilerFamily);
                    targetPlan.compileCommands.push_back(std::move(cmd));
                }

                if (pkgTarget.type == TargetType::staticLibrary)
                {
                    ArchiveCommand archCmd;
                    archCmd.archiver = toolchain.archiver;
                    archCmd.output = targetPlan.artifactOutput;
                    if (toolchain.compilerFamily != CompilerFamily::msvc)
                        archCmd.options = {"rcs"};
                    for (const auto& cCmd : targetPlan.compileCommands)
                    {
                        archCmd.objects.push_back(cCmd.object);
                    }
                    targetPlan.archiveCommand = std::move(archCmd);
                }
                else
                {
                    LinkCommand linkCmd;
                    linkCmd.linker = toolchain.linker;
                    linkCmd.output = targetPlan.artifactOutput;
                    for (const auto& cCmd : targetPlan.compileCommands)
                    {
                        linkCmd.objects.push_back(cCmd.object);
                    }
                    if (pkgTarget.type == TargetType::sharedLibrary)
                    {
                        if (toolchain.compilerFamily == CompilerFamily::msvc)
                            linkCmd.options.push_back("/LD");
                        else
                            linkCmd.options.push_back("-shared");
                    }
                    targetPlan.linkCommand = std::move(linkCmd);
                }

                plan.targetPlans.push_back(std::move(targetPlan));
            }
        }

        // Build project targets
        for (const auto* currentTarget : targetsToBuild)
        {
            SourceScanner scanner;
            auto sources = scanner.scan(
                project.rootDirectory,
                currentTarget->sourcePatterns,
                currentTarget->entry
            );

            if (sources.empty())
            {
                throw std::runtime_error(
                    "no C++ source files found for target '" + currentTarget->name + "'"
                );
            }

            const auto targetBuildDirectory =
                project.rootDirectory /
                ".cippie/build" /
                tripleStr /
                std::string(configuration) /
                currentTarget->name;

            TargetBuildPlan targetPlan;
            targetPlan.targetName = currentTarget->name;
            targetPlan.targetType = currentTarget->type;

            if (currentTarget->type == TargetType::staticLibrary)
            {
                targetPlan.artifactOutput = targetBuildDirectory / "lib" /
                    (toolchain.compilerFamily == CompilerFamily::msvc ? currentTarget->name + ".lib" : "lib" + currentTarget->name + ".a");
            }
            else if (currentTarget->type == TargetType::sharedLibrary)
            {
                targetPlan.artifactOutput = targetBuildDirectory / "lib" /
                    (toolchain.compilerFamily == CompilerFamily::msvc ? currentTarget->name + ".dll" : "lib" + currentTarget->name + ".so");
            }
            else
            {
                auto exeName = currentTarget->name;
                if (toolchain.compilerFamily == CompilerFamily::msvc && exeName.find('.') == std::string::npos)
                    exeName += ".exe";
                targetPlan.artifactOutput = targetBuildDirectory / "bin" / exeName;
            }

            artifactMap[currentTarget->name] = targetPlan.artifactOutput;

            // Collect public includes from project dependencies & resolved packages
            std::vector<std::filesystem::path> propagatedIncludes;
            auto collectPublicIncludes = [&](auto& self, const Target* t) -> void {
                for (const auto& linkName : t->links)
                {
                    auto it = targetMap.find(linkName);
                    if (it != targetMap.end())
                    {
                        for (const auto& pubInc : it->second->publicIncludeDirectories)
                        {
                            addUniquePath(propagatedIncludes, pubInc);
                        }
                        self(self, it->second);
                    }

                    // Package include propagation
                    auto pkgIt = packageGraph.packages.find(linkName);
                    if (pkgIt != packageGraph.packages.end())
                    {
                        auto pkgProjRes = configLoader.load(pkgIt->second.sourceDirectory);
                        if (pkgProjRes.has_value())
                        {
                            for (const auto& pTarget : pkgProjRes->targets)
                            {
                                for (const auto& pubInc : pTarget.publicIncludeDirectories)
                                {
                                    addUniquePath(propagatedIncludes, pubInc);
                                }
                            }
                        }
                    }
                }
            };
            collectPublicIncludes(collectPublicIncludes, currentTarget);

            // Compile commands setup
            for (const auto& source : sources)
            {
                std::filesystem::path relSource;
                std::error_code ec;
                relSource = std::filesystem::relative(source, project.rootDirectory, ec);
                if (ec || relSource.empty() || relSource.string().rfind("..", 0) == 0)
                {
                    relSource = source.filename();
                }

                auto object = targetBuildDirectory / "obj" / relSource;
                object += (toolchain.compilerFamily == CompilerFamily::msvc) ? ".obj" : ".o";

                CompileCommand cmd;
                cmd.compiler = toolchain.cxxCompiler;
                cmd.source = source;
                cmd.object = object;

                for (const auto& inc : currentTarget->includeDirectories)
                {
                    addUniquePath(cmd.includeDirectories, inc);
                }
                for (const auto& pubInc : currentTarget->publicIncludeDirectories)
                {
                    addUniquePath(cmd.includeDirectories, pubInc);
                }
                for (const auto& propInc : propagatedIncludes)
                {
                    addUniquePath(cmd.includeDirectories, propInc);
                }

                cmd.definitions = currentTarget->compileDefinitions;
                cmd.options = {"-std=c++" + std::to_string(project.cppStandard), "-Wall", "-Wextra", "-Wpedantic", "-g"};

                if (currentTarget->type == TargetType::sharedLibrary)
                {
                    cmd.options.push_back("-fPIC");
                }

                for (const auto& opt : currentTarget->compileOptions)
                {
                    cmd.options.push_back(opt);
                }

                translateOptions(cmd.options, toolchain.compilerFamily);
                targetPlan.compileCommands.push_back(std::move(cmd));
            }

            if (currentTarget->type == TargetType::staticLibrary)
            {
                ArchiveCommand archCmd;
                archCmd.archiver = toolchain.archiver;
                archCmd.output = targetPlan.artifactOutput;
                if (toolchain.compilerFamily != CompilerFamily::msvc)
                    archCmd.options = {"rcs"};
                for (const auto& cCmd : targetPlan.compileCommands)
                {
                    archCmd.objects.push_back(cCmd.object);
                }
                targetPlan.archiveCommand = std::move(archCmd);
            }
            else
            {
                LinkCommand linkCmd;
                linkCmd.linker = toolchain.linker;
                linkCmd.output = targetPlan.artifactOutput;

                for (const auto& cCmd : targetPlan.compileCommands)
                {
                    linkCmd.objects.push_back(cCmd.object);
                }

                if (currentTarget->type == TargetType::sharedLibrary)
                {
                    if (toolchain.compilerFamily == CompilerFamily::msvc)
                        linkCmd.options.push_back("/LD");
                    else
                        linkCmd.options.push_back("-shared");
                }

                for (const auto& opt : currentTarget->linkOptions)
                {
                    linkCmd.options.push_back(opt);
                }

                for (const auto& linkName : currentTarget->links)
                {
                    auto it = artifactMap.find(linkName);
                    if (it != artifactMap.end())
                    {
                        const auto& libArtifact = it->second;
                        linkCmd.libraries.push_back(libArtifact.string());

                        auto libDir = libArtifact.parent_path();
                        if (toolchain.compilerFamily != CompilerFamily::msvc && libArtifact.extension() == ".so")
                        {
                            linkCmd.options.push_back("-Wl,-rpath," + libDir.string());
                        }
                    }
                    else
                    {
                        linkCmd.libraries.push_back(linkName);
                    }
                }

                targetPlan.linkCommand = std::move(linkCmd);
            }

            plan.targetPlans.push_back(std::move(targetPlan));
        }

        return plan;
    }
}
