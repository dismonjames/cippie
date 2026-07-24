#pragma once

#include <cippie/core/Result.hpp>
#include <cippie/project/Project.hpp>
#include <cippie/project/Target.hpp>

#include <string_view>

namespace cippie
{
    class TargetSelector
    {
    public:
        TargetSelector() = default;

        [[nodiscard]] Result<const Target*> select(
            const Project& project,
            std::string_view requestedTarget = {}
        ) const;
    };
}
