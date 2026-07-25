#include <cippie/build/BuildEngine.hpp>

#include <cippie/cache/CacheKey.hpp>
#include <cippie/cache/DepFileParser.hpp>
#include <cippie/cache/FileHasher.hpp>
#include <cippie/cache/ManifestStore.hpp>
#include <cippie/process/Process.hpp>
#include <cippie/build/TaskScheduler.hpp>

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace cippie
{
    BuildEngine::BuildEngine(const Logger& logger)
        : logger_(logger)
    {
    }

    bool BuildEngine::execute(
        const BuildPlan& plan,
        const Toolchain& toolchain,
        size_t jobs,
        bool verbose
    ) const
    {
        if (jobs == 0)
        {
            jobs = std::thread::hardware_concurrency();
            if (jobs == 0) jobs = 1;
        }

        // Build execution graph nodes
        std::vector<ExecutionNode> nodes;
        std::unordered_map<std::string, std::string> targetArtifactNodeMap; // targetName -> link/archive nodeId

        for (const auto& tPlan : plan.targetPlans)
        {
            std::vector<std::string> targetCompileNodeIds;

            for (const auto& cmd : tPlan.compileCommands)
            {
                ExecutionNode node;
                node.id = "CXX:" + tPlan.targetName + ":" + cmd.source.string();
                node.kind = NodeKind::compile;
                node.targetName = tPlan.targetName;
                node.compileCommand = cmd;
                node.artifactOutput = tPlan.artifactOutput;

                std::error_code ec;
                auto relSource = std::filesystem::relative(cmd.source, cmd.object.parent_path().parent_path().parent_path(), ec);
                if (ec || relSource.empty() || relSource.string().rfind("..", 0) == 0)
                {
                    relSource = cmd.source.filename();
                }

                auto depDir = cmd.object.parent_path().parent_path() / "dep";
                node.depFilePath = depDir / relSource;
                node.depFilePath += ".d";

                targetCompileNodeIds.push_back(node.id);
                nodes.push_back(std::move(node));
            }

            if (tPlan.archiveCommand.has_value())
            {
                ExecutionNode node;
                node.id = "AR:" + tPlan.targetName;
                node.kind = NodeKind::archive;
                node.targetName = tPlan.targetName;
                node.archiveCommand = *tPlan.archiveCommand;
                node.artifactOutput = tPlan.artifactOutput;
                node.dependencies = targetCompileNodeIds;

                targetArtifactNodeMap[tPlan.targetName] = node.id;
                nodes.push_back(std::move(node));
            }
            else if (tPlan.linkCommand.has_value())
            {
                ExecutionNode node;
                node.id = "LINK:" + tPlan.targetName;
                node.kind = NodeKind::link;
                node.targetName = tPlan.targetName;
                node.linkCommand = *tPlan.linkCommand;
                node.artifactOutput = tPlan.artifactOutput;
                node.dependencies = targetCompileNodeIds;

                // Add dependent target library node IDs as dependencies
                for (const auto& depTargetPlan : plan.targetPlans)
                {
                    if (depTargetPlan.targetName != tPlan.targetName)
                    {
                        auto it = targetArtifactNodeMap.find(depTargetPlan.targetName);
                        if (it != targetArtifactNodeMap.end())
                        {
                            node.dependencies.push_back(it->second);
                        }
                    }
                }

                targetArtifactNodeMap[tPlan.targetName] = node.id;
                nodes.push_back(std::move(node));
            }
        }

        // Shared build data & mutexes
        std::mutex outputMutex;
        std::mutex manifestMutex;
        std::mutex fileHashMutex;
        std::unordered_map<std::filesystem::path, std::string> fileHashMemo;
        std::unordered_map<std::string, std::vector<CacheEntry>> manifestMap; // targetName -> entries
        ManifestStore manifestStore;
        FileHasher hasher;
        DepFileParser depParser;
        Process process;

        auto getManifestPath = [&](const std::string& tName, const std::filesystem::path& artOutput) -> std::filesystem::path {
            auto cur = artOutput.parent_path();
            while (!cur.empty() && cur != cur.root_path())
            {
                if (cur.filename() == ".cippie")
                {
                    return cur / "manifests" / toolchain.target.toString() / "debug" / tName / "manifest.txt";
                }
                cur = cur.parent_path();
            }
            return artOutput.parent_path().parent_path() / "manifests" / "manifest.txt";
        };

        // Load manifests per target
        for (const auto& tPlan : plan.targetPlans)
        {
            auto manifestPath = getManifestPath(tPlan.targetName, tPlan.artifactOutput);
            manifestMap[tPlan.targetName] = manifestStore.load(manifestPath);
        }

        // Count non-cached total steps
        std::atomic<size_t> totalSteps{nodes.size()};
        std::atomic<size_t> currentStep{0};

        TaskScheduler scheduler;

        bool buildSuccess = scheduler.execute(nodes, jobs, [&](ExecutionNode& node) -> bool {
            if (node.kind == NodeKind::compile)
            {
                const auto& cmd = *node.compileCommand;

                std::string sourceHash;
                {
                    std::lock_guard<std::mutex> lock(fileHashMutex);
                    auto hRes = hasher.hashFile(cmd.source, &fileHashMemo);
                    if (!hRes.has_value())
                    {
                        node.rebuildReason = "failed to hash source file: " + cmd.source.string();
                    }
                    else
                    {
                        sourceHash = *hRes;
                    }
                }

                // Load existing manifest entries for target
                std::vector<CacheEntry> targetEntries;
                {
                    std::lock_guard<std::mutex> lock(manifestMutex);
                    targetEntries = manifestMap[node.targetName];
                }

                const CacheEntry* prevEntry = nullptr;
                for (const auto& e : targetEntries)
                {
                    if (e.sourcePath == cmd.source)
                    {
                        prevEntry = &e;
                        break;
                    }
                }

                // Check header dependencies from previous entry
                std::vector<std::pair<std::filesystem::path, std::string>> headerDeps;
                bool depsValid = true;

                if (prevEntry != nullptr)
                {
                    for (const auto& depPair : prevEntry->dependencies)
                    {
                        std::error_code ec;
                        if (!std::filesystem::is_regular_file(depPair.first, ec))
                        {
                            depsValid = false;
                            node.rebuildReason = "header dependency missing: " + depPair.first.string();
                            break;
                        }

                        std::string headerHash;
                        {
                            std::lock_guard<std::mutex> lock(fileHashMutex);
                            auto hRes = hasher.hashFile(depPair.first, &fileHashMemo);
                            if (!hRes.has_value())
                            {
                                depsValid = false;
                                node.rebuildReason = "header dependency read failed: " + depPair.first.string();
                                break;
                            }
                            headerHash = *hRes;
                        }

                        headerDeps.emplace_back(depPair.first, headerHash);
                    }
                }

                bool isPic = (node.targetName.find("shared") != std::string::npos);
                std::string currentKey = CacheKeyBuilder::buildCompileKey(
                    cmd.compiler.string(),
                    toolchain.version,
                    toString(toolchain.compilerFamily),
                    toolchain.target.toString(),
                    "debug",
                    23,
                    isPic,
                    cmd.source,
                    sourceHash,
                    cmd.includeDirectories,
                    cmd.definitions,
                    cmd.options,
                    headerDeps
                );

                std::error_code ec;
                bool objectExists = std::filesystem::is_regular_file(cmd.object, ec);

                if (objectExists && prevEntry != nullptr && depsValid && prevEntry->cacheKey == currentKey)
                {
                    node.state = NodeState::cached;
                    if (verbose)
                    {
                        std::lock_guard<std::mutex> lock(outputMutex);
                        size_t step = ++currentStep;
                        std::cout << "[" << step << "/" << totalSteps << "] CACHED "
                                  << std::filesystem::relative(cmd.source, std::filesystem::current_path(), ec).string()
                                  << "\n";
                    }
                    return true;
                }



                // Execute compiler process
                std::filesystem::create_directories(cmd.object.parent_path());
                std::filesystem::create_directories(node.depFilePath.parent_path());

                ProcessRequest req;
                req.executable = cmd.compiler;
                req.captureOutput = true;

                for (const auto& opt : cmd.options) req.arguments.push_back(opt);
                for (const auto& def : cmd.definitions) req.arguments.push_back("-D" + def);
                for (const auto& inc : cmd.includeDirectories) req.arguments.push_back("-I" + inc.string());

                req.arguments.push_back("-MMD");
                req.arguments.push_back("-MP");
                req.arguments.push_back("-MF");
                req.arguments.push_back(node.depFilePath.string());
                req.arguments.push_back("-MT");
                req.arguments.push_back(cmd.object.string());

                req.arguments.push_back("-c");
                req.arguments.push_back(cmd.source.string());
                req.arguments.push_back("-o");
                req.arguments.push_back(cmd.object.string());

                auto res = process.run(req);

                {
                    std::lock_guard<std::mutex> lock(outputMutex);
                    size_t step = ++currentStep;
                    auto displaySource = std::filesystem::relative(cmd.source, std::filesystem::current_path(), ec);
                    if (ec || displaySource.empty()) displaySource = cmd.source;

                    std::error_code ec2;
                    auto displayObj = std::filesystem::relative(cmd.object, std::filesystem::current_path(), ec2);
                    if (ec2 || displayObj.empty()) displayObj = cmd.object;

                    logger_.buildStep(step, totalSteps, "CXX", displaySource.string(), displayObj.string());

                    if (res.exitCode != 0)
                    {
                        if (!res.stdoutOutput.empty()) std::cout << res.stdoutOutput;
                        if (!res.stderrOutput.empty()) std::cerr << res.stderrOutput;
                        return false;
                    }

                    if (verbose)
                    {
                        if (!res.stdoutOutput.empty()) std::cout << res.stdoutOutput;
                        if (!res.stderrOutput.empty()) std::cerr << res.stderrOutput;
                    }
                }

                // Parse generated .d depfile
                auto parsedDepsRes = depParser.parse(node.depFilePath);
                std::vector<std::pair<std::filesystem::path, std::string>> finalHeaderDeps;

                if (parsedDepsRes.has_value())
                {
                    for (const auto& hPath : *parsedDepsRes)
                    {
                        std::lock_guard<std::mutex> lock(fileHashMutex);
                        auto hRes = hasher.hashFile(hPath, &fileHashMemo);
                        if (hRes.has_value())
                        {
                            finalHeaderDeps.emplace_back(hPath, *hRes);
                        }
                    }
                }

                std::string finalKey = CacheKeyBuilder::buildCompileKey(
                    cmd.compiler.string(),
                    toolchain.version,
                    toString(toolchain.compilerFamily),
                    toolchain.target.toString(),
                    "debug",
                    23,
                    isPic,
                    cmd.source,
                    sourceHash,
                    cmd.includeDirectories,
                    cmd.definitions,
                    cmd.options,
                    finalHeaderDeps
                );

                // Update manifest atomically
                {
                    std::lock_guard<std::mutex> lock(manifestMutex);
                    auto& entries = manifestMap[node.targetName];
                    bool updated = false;
                    for (auto& e : entries)
                    {
                        if (e.sourcePath == cmd.source)
                        {
                            e.cacheKey = finalKey;
                            e.objectPath = cmd.object;
                            e.depFilePath = node.depFilePath;
                            e.dependencies = finalHeaderDeps;
                            updated = true;
                            break;
                        }
                    }
                    if (!updated)
                    {
                        entries.push_back(CacheEntry{
                            .targetName = node.targetName,
                            .sourcePath = cmd.source,
                            .objectPath = cmd.object,
                            .depFilePath = node.depFilePath,
                            .cacheKey = finalKey,
                            .dependencies = finalHeaderDeps
                        });
                    }

                    auto manifestPath = getManifestPath(node.targetName, node.artifactOutput);
                    (void)manifestStore.save(manifestPath, entries);
                }

                return true;
            }
            else if (node.kind == NodeKind::archive)
            {
                const auto& archCmd = *node.archiveCommand;

                std::vector<std::string> objHashes;
                for (const auto& obj : archCmd.objects)
                {
                    std::lock_guard<std::mutex> lock(fileHashMutex);
                    auto hRes = hasher.hashFile(obj, &fileHashMemo);
                    if (hRes.has_value()) objHashes.push_back(*hRes);
                }

                std::error_code ec;
                if (std::filesystem::is_regular_file(archCmd.output, ec))
                {
                    // Check if inputs were cached
                    bool allInputsCached = true;
                    for (const auto& depId : node.dependencies)
                    {
                        for (const auto& n : nodes)
                        {
                            if (n.id == depId && n.state != NodeState::cached)
                            {
                                allInputsCached = false;
                                break;
                            }
                        }
                    }

                    if (allInputsCached)
                    {
                        node.state = NodeState::cached;
                        if (verbose)
                        {
                            std::lock_guard<std::mutex> lock(outputMutex);
                            size_t step = ++currentStep;
                            std::cout << "[" << step << "/" << totalSteps << "] CACHED "
                                      << archCmd.output.filename().string() << "\n";
                        }
                        return true;
                    }
                }

                std::filesystem::create_directories(archCmd.output.parent_path());

                ProcessRequest req;
                req.executable = archCmd.archiver;
                req.captureOutput = true;

                for (const auto& opt : archCmd.options) req.arguments.push_back(opt);
                req.arguments.push_back(archCmd.output.string());
                for (const auto& obj : archCmd.objects) req.arguments.push_back(obj.string());

                auto res = process.run(req);

                {
                    std::lock_guard<std::mutex> lock(outputMutex);
                    size_t step = ++currentStep;
                    logger_.buildStep(step, totalSteps, "AR", archCmd.output.filename().string());

                    if (res.exitCode != 0)
                    {
                        if (!res.stdoutOutput.empty()) std::cout << res.stdoutOutput;
                        if (!res.stderrOutput.empty()) std::cerr << res.stderrOutput;
                        return false;
                    }
                }

                return true;
            }
            else if (node.kind == NodeKind::link)
            {
                const auto& linkCmd = *node.linkCommand;

                std::error_code ec;
                if (std::filesystem::is_regular_file(linkCmd.output, ec))
                {
                    bool allInputsCached = true;
                    for (const auto& depId : node.dependencies)
                    {
                        for (const auto& n : nodes)
                        {
                            if (n.id == depId && n.state != NodeState::cached)
                            {
                                allInputsCached = false;
                                break;
                            }
                        }
                    }

                    if (allInputsCached)
                    {
                        node.state = NodeState::cached;
                        if (verbose)
                        {
                            std::lock_guard<std::mutex> lock(outputMutex);
                            size_t step = ++currentStep;
                            std::cout << "[" << step << "/" << totalSteps << "] CACHED "
                                      << node.targetName << "\n";
                        }
                        return true;
                    }
                }

                std::filesystem::create_directories(linkCmd.output.parent_path());

                ProcessRequest req;
                req.executable = linkCmd.linker;
                req.captureOutput = true;

                for (const auto& opt : linkCmd.options) req.arguments.push_back(opt);
                for (const auto& obj : linkCmd.objects) req.arguments.push_back(obj.string());
                for (const auto& lib : linkCmd.libraries) req.arguments.push_back(lib);
                req.arguments.push_back("-o");
                req.arguments.push_back(linkCmd.output.string());

                auto res = process.run(req);

                {
                    std::lock_guard<std::mutex> lock(outputMutex);
                    size_t step = ++currentStep;
                    std::error_code ecLink;
                    auto displayLinkDir = std::filesystem::relative(
                        linkCmd.output.parent_path(), std::filesystem::current_path(), ecLink);
                    if (ecLink || displayLinkDir.empty()) displayLinkDir = linkCmd.output.parent_path();
                    logger_.buildStep(step, totalSteps, "LINK", displayLinkDir.string(),
                                       linkCmd.output.filename().string());

                    if (res.exitCode != 0)
                    {
                        if (!res.stdoutOutput.empty()) std::cout << res.stdoutOutput;
                        if (!res.stderrOutput.empty()) std::cerr << res.stderrOutput;
                        return false;
                    }
                }

                return true;
            }

            return false;
        });

        return buildSuccess;
    }
}
