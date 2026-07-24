#include <cippie/project/TargetSelector.hpp>

#include <sstream>
#include <vector>

namespace cippie
{
    Result<const Target*> TargetSelector::select(
        const Project& project,
        std::string_view requestedTarget
    ) const
    {
        if (project.targets.empty())
        {
            return std::unexpected(Error{
                .code = ErrorCode::targetNotFound,
                .message = "project has no targets",
                .location = std::nullopt,
                .notes = {}
            });
        }

        if (!requestedTarget.empty())
        {
            for (const auto& target : project.targets)
            {
                if (target.name == requestedTarget)
                {
                    return &target;
                }
            }

            return std::unexpected(Error{
                .code = ErrorCode::targetNotFound,
                .message = "target not found: " + std::string(requestedTarget),
                .location = std::nullopt,
                .notes = {}
            });
        }

        if (project.defaultTarget.has_value() && !project.defaultTarget->empty())
        {
            for (const auto& target : project.targets)
            {
                if (target.name == *project.defaultTarget)
                {
                    return &target;
                }
            }

            return std::unexpected(Error{
                .code = ErrorCode::targetNotFound,
                .message = "defaultTarget not found: " + *project.defaultTarget,
                .location = std::nullopt,
                .notes = {}
            });
        }

        std::vector<const Target*> runnableTargets;
        for (const auto& target : project.targets)
        {
            if (target.type == TargetType::executable || target.type == TargetType::test)
            {
                runnableTargets.push_back(&target);
            }
        }

        if (runnableTargets.size() == 1)
        {
            return runnableTargets.front();
        }

        if (runnableTargets.empty())
        {
            if (project.targets.size() == 1)
            {
                return &project.targets.front();
            }

            return std::unexpected(Error{
                .code = ErrorCode::targetNotFound,
                .message = "no runnable targets found",
                .location = std::nullopt,
                .notes = {}
            });
        }

        std::ostringstream ss;
        ss << "multiple runnable targets found\n\n";
        for (const auto* target : runnableTargets)
        {
            ss << "  " << target->name << "\n";
        }
        ss << "\nuse:\n  cippie run <target>";

        return std::unexpected(Error{
            .code = ErrorCode::targetNotFound,
            .message = ss.str(),
            .location = std::nullopt,
            .notes = {}
        });
    }
}
