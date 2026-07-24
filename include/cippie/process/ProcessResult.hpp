#pragma once

#include <string>

namespace cippie
{
    struct ProcessResult
    {
        int exitCode{0};
        bool exitedNormally{true};
        std::string stdoutOutput;
        std::string stderrOutput;
    };
}
