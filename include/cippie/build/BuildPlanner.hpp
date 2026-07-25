#pragma once

#include <cippie/build/BuildPlan.hpp>
#include <cippie/project/Project.hpp>
#include <cippie/toolchain/Toolchain.hpp>

#include <string_view>

namespace cippie
{
    class BuildPlanner
    {
    public:
        BuildPlanner() = default;

        [[nodiscard]] BuildPlan create(
            const Project& project,
            const Target& target,
            const Toolchain& toolchain,
            std::string_view configuration = "debug"
        ) const;
    };
}
