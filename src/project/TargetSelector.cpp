#include <cippie/project/TargetSelector.hpp>

#include <sstream>
#include <vector>

namespace cippie
{
    Result<const Target*> TargetSelector::selectForBuild(
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

        if (project.targets.size() == 1)
        {
            return &project.targets.front();
        }

        // Search for unambiguous executable target
        std::vector<const Target*> executables;
        for (const auto& target : project.targets)
        {
            if (target.type == TargetType::executable)
            {
                executables.push_back(&target);
            }
        }

        if (executables.size() == 1)
        {
            return executables.front();
        }

        std::ostringstream ss;
        ss << "multiple targets found in project\n\n";
        for (const auto& target : project.targets)
        {
            ss << "  " << target.name << "\n";
        }
        ss << "\nuse:\n  cippie build <target>";

        return std::unexpected(Error{
            .code = ErrorCode::targetNotFound,
            .message = ss.str(),
            .location = std::nullopt,
            .notes = {}
        });
    }

    Result<const Target*> TargetSelector::selectForRun(
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
                    if (target.type == TargetType::staticLibrary ||
                        target.type == TargetType::sharedLibrary)
                    {
                        return std::unexpected(Error{
                            .code = ErrorCode::targetNotFound,
                            .message = "target '" + std::string(requestedTarget) +
                                "' is a library and cannot be run",
                            .location = std::nullopt,
                            .notes = {}
                        });
                    }
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
                    if (target.type == TargetType::staticLibrary ||
                        target.type == TargetType::sharedLibrary)
                    {
                        return std::unexpected(Error{
                            .code = ErrorCode::targetNotFound,
                            .message = "defaultTarget '" + *project.defaultTarget +
                                "' is a library and cannot be run",
                            .location = std::nullopt,
                            .notes = {}
                        });
                    }
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
            if (target.type == TargetType::executable)
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

    Result<std::vector<const Target*>> TargetSelector::selectForTest(
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

        std::vector<const Target*> selected;

        if (!requestedTarget.empty())
        {
            for (const auto& target : project.targets)
            {
                if (target.name == requestedTarget)
                {
                    selected.push_back(&target);
                    return selected;
                }
            }

            return std::unexpected(Error{
                .code = ErrorCode::targetNotFound,
                .message = "test target not found: " + std::string(requestedTarget),
                .location = std::nullopt,
                .notes = {}
            });
        }

        for (const auto& target : project.targets)
        {
            if (target.type == TargetType::test)
            {
                selected.push_back(&target);
            }
        }

        if (selected.empty())
        {
            // If no explicit test targets, fall back to executable targets
            for (const auto& target : project.targets)
            {
                if (target.type == TargetType::executable)
                {
                    selected.push_back(&target);
                }
            }
        }

        if (selected.empty())
        {
            return std::unexpected(Error{
                .code = ErrorCode::targetNotFound,
                .message = "no test targets found",
                .location = std::nullopt,
                .notes = {}
            });
        }

        return selected;
    }
}
