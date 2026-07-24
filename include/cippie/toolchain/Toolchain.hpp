#pragma once

#include <cippie/toolchain/CompilerFamily.hpp>
#include <cippie/toolchain/TargetTriple.hpp>

#include <filesystem>
#include <string>

namespace cippie
{
    struct Toolchain
    {
        std::string name{"default"};
        CompilerFamily compilerFamily{CompilerFamily::gcc};

        std::filesystem::path cCompiler{"cc"};
        std::filesystem::path cxxCompiler{"c++"};
        std::filesystem::path linker{"c++"};
        std::filesystem::path archiver{"ar"};

        std::string version;
        TargetTriple host{TargetTriple::detectHost()};
        TargetTriple target{TargetTriple::detectHost()};
    };
}
