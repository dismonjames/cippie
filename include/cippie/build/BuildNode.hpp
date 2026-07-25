#pragma once

#include <cippie/project/Target.hpp>
#include <string>
#include <vector>

namespace cippie
{
    struct BuildNode
    {
        const Target* target{nullptr};
        std::vector<std::string> dependencies;
    };
}
