#pragma once

#include <cippie/process/ProcessRequest.hpp>
#include <cippie/process/ProcessResult.hpp>

namespace cippie
{
    class ProcessRunner
    {
    public:
        ProcessRunner() = default;
        virtual ~ProcessRunner() = default;

        [[nodiscard]] virtual ProcessResult run(
            const ProcessRequest& request
        ) const;
    };
}
