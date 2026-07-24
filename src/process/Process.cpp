#include <cippie/process/Process.hpp>

namespace cippie
{
    ProcessResult Process::run(const ProcessRequest& request) const
    {
        ProcessRunner runner;
        return runner.run(request);
    }
}
