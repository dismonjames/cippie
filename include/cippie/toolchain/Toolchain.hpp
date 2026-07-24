#pragma once

#include <string>

namespace cippie
{
    struct Toolchain
    {
        std::string cxxCompiler{"c++"};
        std::string linker{"c++"};
    };
}
