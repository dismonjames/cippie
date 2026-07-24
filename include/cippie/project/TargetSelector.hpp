#pragma once

#include <cippie/core/Result.hpp>
#include <cippie/project/Project.hpp>
#include <cippie/project/Target.hpp>

#include <string_view>
#include <vector>

namespace cippie
{
    class TargetSelector
    {
    public:
        TargetSelector() = default;

        [[nodiscard]] Result<const Target*> selectForBuild(
            const Project& project,
            std::string_view requestedTarget = {}
        ) const;

        [[nodiscard]] Result<const Target*> selectForRun(
            const Project& project,
            std::string_view requestedTarget = {}
        ) const;

        [[nodiscard]] Result<std::vector<const Target*>> selectForTest(
            const Project& project,
            std::string_view requestedTarget = {}
        ) const;

        // Legacy compatibility alias
        [[nodiscard]] Result<const Target*> select(
            const Project& project,
            std::string_view requestedTarget = {}
        ) const
        {
            return selectForBuild(project, requestedTarget);
        }
    };
}
