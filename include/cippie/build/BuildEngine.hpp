#pragma once

#include <cippie/build/BuildPlan.hpp>
#include <cippie/build/ExecutionNode.hpp>
#include <cippie/diagnostics/Logger.hpp>
#include <cippie/toolchain/Toolchain.hpp>

#include <mutex>
#include <string>

namespace cippie
{
    class BuildEngine
    {
    public:
        explicit BuildEngine(const Logger& logger);

        [[nodiscard]] bool execute(
            const BuildPlan& plan,
            const Toolchain& toolchain,
            size_t jobs = 0,
            bool verbose = false
        ) const;

    private:
        const Logger& logger_;
    };
}
