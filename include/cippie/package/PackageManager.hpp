#pragma once

#include <cippie/project/Project.hpp>

namespace cippie
{
    class PackageManager
    {
    public:
        void restore(const Project& project) const;
    };
}
