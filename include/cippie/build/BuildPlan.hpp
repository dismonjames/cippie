#pragma once

#include <cippie/build/ArchiveCommand.hpp>
#include <cippie/build/CompileCommand.hpp>
#include <cippie/build/LinkCommand.hpp>
#include <cippie/project/TargetType.hpp>

#include <optional>
#include <string>
#include <vector>

namespace cippie
{
    struct TargetBuildPlan
    {
        std::string targetName;
        TargetType targetType{TargetType::executable};
        std::vector<CompileCommand> compileCommands;
        std::optional<ArchiveCommand> archiveCommand;
        std::optional<LinkCommand> linkCommand;
        std::filesystem::path artifactOutput;
    };

    struct BuildPlan
    {
        std::string rootTargetName;
        std::vector<TargetBuildPlan> targetPlans;
    };
}
