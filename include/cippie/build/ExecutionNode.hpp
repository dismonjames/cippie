#pragma once

#include <cippie/build/ArchiveCommand.hpp>
#include <cippie/build/CompileCommand.hpp>
#include <cippie/build/LinkCommand.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace cippie
{
    enum class NodeKind
    {
        compile,
        archive,
        link,
        test
    };

    enum class NodeState
    {
        pending,
        ready,
        running,
        cached,
        succeeded,
        failed,
        blocked
    };

    struct ExecutionNode
    {
        std::string id;
        NodeKind kind{NodeKind::compile};
        std::string targetName;
        NodeState state{NodeState::pending};

        std::vector<std::string> dependencies; // node IDs required before running
        std::vector<std::string> dependents;   // node IDs waiting on this node

        std::optional<CompileCommand> compileCommand;
        std::optional<ArchiveCommand> archiveCommand;
        std::optional<LinkCommand> linkCommand;

        std::filesystem::path depFilePath;
        std::filesystem::path artifactOutput;
        std::string rebuildReason;
    };
}
